#include "cpu.h"

t_log* logger;
t_config* cpu_config;
uint32_t inputs_tlb;
int instruction_delay,
	interruption_quantum,
	interruption_io_pf,
	socket_cpu_dispatch,
	socket_kernel_dispatch,
	socket_memoria;
char* replace_tlb;
pthread_t thread_interrupt;
//sem_t sem_interrupt;



int main(int argc, char **argv) {

	char* cpu_config_path = argv[1]; // Obtengo path de config

	setup(cpu_config_path);	// Paso el path de cfg por parametro

	connections();  // FUNCION INICIALIZAR DISPATCH E INTERRUPT

	instruction_cycle();  // RECIBO -> CICLO -> ENVIO y repito

	free_memory();   // Valgrind te necesito

	return EXIT_SUCCESS;
}



void setup (char* path){

	// Logger

	logger = log_create("cpu.log", "CPU", true, LOG_LEVEL_TRACE);

	// Creo config

	cpu_config = config_create(path);

	// Error por si no se paso bien el argumento

	if(cpu_config == NULL) {
		log_error(logger, "No se pudo abrir la config de CPU");
		exit(EXIT_FAILURE);
	}

	// Obtengo datos de la config

	replace_tlb = config_get_string_value(cpu_config, "REEMPLAZO_TLB");
	log_trace(logger, "REEMPLAZO_TLB: %s", replace_tlb);

	inputs_tlb = config_get_int_value(cpu_config, "ENTRADAS_TLB");
	log_trace(logger, "ENTRADAS_TLB: %d", inputs_tlb);

	instruction_delay = config_get_int_value(cpu_config, "RETARDO_INSTRUCCION");
	log_trace(logger, "RETARDO_INSTRUCCION: %i ms", instruction_delay);

}



void connections(){

	socket_memoria = start_client_module("MEMORIA_CPU");	  // CONECTAR A MEMORIA

	pthread_create(&thread_interrupt, NULL, start_interrupt, NULL); // CONECTAR A KERNEL INTERRUPT (THREAD)

	socket_cpu_dispatch = start_server_module("CPU_DISPATCH");			// CONECTAR A KERNEL DISPATCH
	log_trace(logger,"Esperando conexion con Kernel desde DISPATCH");	// QUEDA BLOQUEADO HASTA TENER CONEXIÓN CON KERNEL_DISPATCH
	socket_kernel_dispatch = accept(socket_cpu_dispatch, NULL, NULL);
	log_trace(logger, "Conexion con kernel dispatch: %i", socket_kernel_dispatch);

	int memory_size = recv_memory_size(socket_memoria);
	int page_size = recv_page_size(socket_memoria);

	log_trace(logger, "Memory Size: %i", memory_size);
	log_trace(logger, "Page Size: %i", page_size);

}



void instruction_cycle(){

	while(true){

		t_pcb* pcb = recv_pcb(socket_kernel_dispatch);
		interruption_quantum = NO_INTERRUPT;
		pcb->interrupt_type = NO_INTERRUPT;
		interruption_io_pf = NO_INTERRUPT;
		log_trace(logger, "PCB RECIBIDO - Ciclo de instruccion ejecutando");
		char* device;
		int io_value;
		while(pcb->interrupt_type == NO_INTERRUPT){

			log_trace(logger, "FETCH");	// FETCH

			t_instruction* instruction_fetched = list_get(pcb->instructions,pcb->program_counter);

			log_trace(logger, "DECODE"); // DECODE & EXECUTE

			t_parameter* param1;
			t_parameter* param2;

			if(instruction_fetched->instruction != EXIT){

				param1 = (t_parameter*)list_get(instruction_fetched->parameters,0);
				param2 = (t_parameter*)list_get(instruction_fetched->parameters,1);

			}

			switch(instruction_fetched->instruction){
				case SET:

					set_execute(pcb, (t_register)param1->parameter, (uint32_t)param2->parameter);
					log_info(logger, "PID: %i - Ejecutando: %s - %s - %i", pcb->id, t_instruction_type_string[instruction_fetched->instruction], t_register_string[(int)param1->parameter], (int)param2->parameter);
					pcb->program_counter++;

					break;
				case ADD:

					add_execute(pcb, (t_register)param1->parameter, (t_register)param2->parameter);
					log_info(logger, "PID: %i - Ejecutando: %s - %s - %s", pcb->id, t_instruction_type_string[instruction_fetched->instruction], t_register_string[(int)param1->parameter], t_register_string[(int)param2->parameter]);
					pcb->program_counter++;

					break;
				case MOV_IN:

					//t_register reg1 = list_get(instruction_fetched->parameters,0);
					//uint32_t param1 = list_get(instruction_fetched->parameters,1);
					//mov_in_execute(pcb, reg1, param1);
					log_info(logger, "PID: %i - Ejecutando: %s - %s - %i", pcb->id, t_instruction_type_string[instruction_fetched->instruction], t_register_string[(int)param1->parameter], (int)param2->parameter);
					pcb->program_counter++;

					break;
				case MOV_OUT:

					//uint32_t param1 = list_get(instruction_fetched->parameters,0);
					//t_register reg1 = list_get(instruction_fetched->parameters,1);
					log_info(logger, "PID: %i - Ejecutando: %s - %i - %s", pcb->id, t_instruction_type_string[instruction_fetched->instruction], (int)param1->parameter, t_register_string[(int)param2->parameter]);
					pcb->program_counter++;

					break;
				case IO:

					device = (char*)param1->parameter;
					io_value = (int)param2->parameter;
					interruption_io_pf = INT_IO;
					log_info(logger, "PID: %i - Ejecutando: %s - %s - %i", pcb->id, t_instruction_type_string[instruction_fetched->instruction], (char*)param1->parameter, (int)param2->parameter);
					pcb->program_counter++;

					break;
				case EXIT:
					// Guardar directo en pcb para hacer check al final
					log_info(logger, "PID: %i - Ejecutando: %s", pcb->id, t_instruction_type_string[instruction_fetched->instruction]);
					pcb->interrupt_type = EXECUTION_FINISHED;

					break;
			}

			log_trace(logger, "CHECK INTERRUPT"); // CHECK INTERRUPT, HECHO ASI POR CONDICIONES DE CARRERA    EXECUTION_FINISHED > INT_IO = PAGE_FAULT > INT_QUANTUM

			if(pcb->interrupt_type != EXECUTION_FINISHED){
				if(interruption_io_pf == NO_INTERRUPT){
					pcb->interrupt_type = interruption_quantum;
				}
				else{
					pcb->interrupt_type = interruption_io_pf;
				}
			}
		}
		// Mando PCB, a veces con mas info si es INT por IO o PF
		switch(pcb->interrupt_type){
			case INT_IO:
				send_pcb_io(socket_kernel_dispatch, pcb, device, io_value);
				break;
			case INT_PAGE_FAULT:
				send_pcb(socket_kernel_dispatch, pcb);
				break;
			default:
				send_pcb(socket_kernel_dispatch, pcb);
				break;
		}
		log_pcb(logger, pcb);
		log_trace(logger, "PCB ENVIADO - A la espera de otro proceso");
	}
}



void free_memory(){

	config_destroy(cpu_config);
	log_destroy(logger);

}



void* start_interrupt(void* arg) {
	// FUNCION PARA THREAD

	int socket_cpu_interrupt = start_server_module("CPU_INTERRUPT");
	log_trace(logger,"Esperando conexion con Kernel desde INTERRUPT");
	int socket_kernel_interrupt = accept(socket_cpu_interrupt, NULL, NULL);
	log_trace(logger, "Conexion con kernel interrupt: %i", socket_kernel_interrupt);

	while(true){
		// Introducir recibir interrupcion
		recv_interrupt(socket_kernel_interrupt);
		interruption_quantum = INT_QUANTUM;
	}
}



void set_execute(t_pcb* pcb, t_register reg1, uint32_t param1){

	pcb->registers[reg1] = param1;
	sleep((int)(instruction_delay * 0.001));

}



void add_execute(t_pcb* pcb, t_register reg1, t_register reg2){

	pcb->registers[reg1] = pcb->registers[reg1]+pcb->registers[reg2];
	sleep((int)(instruction_delay * 0.001));

}



/*void mov_in_execute(t_pcb* pcb, t_register reg1, uint32_t param1){




}*/


/*
   HANDSHAKE: Hara falta que la memoria envie los datos de config necesarios (cant pag/seg y tam pag)
   SECCION TLB, la idea es tener un valor de mas en la estructura TLB, ese valor es un int
   que sirve a la hora de hacer FIFO o LRU, y este se maneja distinto dependiendo si es uno u otro
   FIFO: cada vez que se llena una entrada se asigna un numero 1+ que el anterior, si se repite un
   presente no cambia nada. AL reemplazar busco el menor. Nada de timestamp
   LRU: cada vez que se llena una entrada se asigna un numero 1+ que el anterior, pero se repite un
   presente el valor de ref se cambia por uno nuevo. Al reemplazar busco el menor. Nada de timestamp

   SECCION MMU, es una función. Sería invocada cada vez que se recibe mov_in o mov_out. Se toma el parametro
   que contenga la dir logica
*/


