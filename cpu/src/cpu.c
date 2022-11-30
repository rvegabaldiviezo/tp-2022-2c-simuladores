#include "cpu.h"

t_log* logger;
t_config* cpu_config;
t_list* tlb;
int instruction_delay,
	interruption_quantum,
	interruption_io_pf,
	socket_cpu_dispatch,
	socket_kernel_dispatch,
	socket_memoria,
	socket_memoria_tlb,
	inputs_table_memory,
	page_size,
	inputs_tlb,
	pf_segment,
	pf_page;
long timestamp;
char* replace_tlb;
pthread_t thread_interrupt;
pthread_t thread_tlb_consistency;
pthread_mutex_t sem_mutex_tlb;



int main(int argc, char **argv) {

	setup(argv);	// Paso el argv por parametro

	connections();  // FUNCION INICIALIZAR DISPATCH E INTERRUPT

	instruction_cycle();  // RECIBO -> CICLO -> ENVIO y repito

	free_memory();   // Valgrind te necesito

	return EXIT_SUCCESS;
}



void setup (char **argv){

	// Logger

	logger = log_create("cpu.log", "CPU", true, LOG_LEVEL_INFO);

	// Creo config

	char* cpu_config_path = argv[1];

	cpu_config = config_create(cpu_config_path);

	// Error por si no se paso bien el argumento

	if(cpu_config == NULL) {
		log_error(logger, "No se pudo abrir la config de CPU");
		exit(EXIT_FAILURE);
	}

	// Obtengo datos de la config

	replace_tlb = config_get_string_value(cpu_config, "REEMPLAZO_TLB");
	log_trace(logger, "REEMPLAZO_TLB: %s", replace_tlb);

	inputs_tlb = config_get_int_value(cpu_config, "ENTRADAS_TLB");
	log_trace(logger, "ENTRADAS_TLB: %i", inputs_tlb);

	instruction_delay = config_get_int_value(cpu_config, "RETARDO_INSTRUCCION");
	log_trace(logger, "RETARDO_INSTRUCCION: %i ms", instruction_delay);

	pthread_mutex_init(&sem_mutex_tlb, NULL);  // Inicializo mutex de TLB

}



void connections(){

	socket_memoria = start_client_module("MEMORIA_CPU");	  // CONECTAR A MEMORIA

	pthread_create(&thread_interrupt, NULL, start_interrupt, NULL); // CONECTAR A KERNEL INTERRUPT (THREAD)

	pthread_create(&thread_tlb_consistency, NULL, consistency_check, NULL); // CONECTAR A MEMORIA TLB (para eliminar inconsistencia)

	socket_cpu_dispatch = start_server_module("CPU_DISPATCH");			// CONECTAR A KERNEL DISPATCH
	log_trace(logger,"Esperando conexion con Kernel desde DISPATCH");	// QUEDA BLOQUEADO HASTA TENER CONEXIÓN CON KERNEL_DISPATCH
	socket_kernel_dispatch = accept(socket_cpu_dispatch, NULL, NULL);
	log_trace(logger, "Conexion con kernel dispatch: %i", socket_kernel_dispatch);

	inputs_table_memory = recv_int(socket_memoria);
	page_size = recv_int(socket_memoria);

	log_trace(logger, "Memory Size: %i", inputs_table_memory);
	log_trace(logger, "Page Size: %i", page_size);

}



void instruction_cycle(){

	tlb = list_create(); // Creo la tlb

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

			//log_pcb(logger, pcb);
			switch(instruction_fetched->instruction){
				case SET:

					log_info(logger, "PID: %i - Ejecutando: %s - %s - %i", pcb->id, t_instruction_type_string[instruction_fetched->instruction], t_register_string[(int)param1->parameter], (int)param2->parameter);
					set_execute(pcb, (t_register)param1->parameter, (uint32_t)param2->parameter);
					pcb->program_counter++;

					break;
				case ADD:

					log_info(logger, "PID: %i - Ejecutando: %s - %s - %s", pcb->id, t_instruction_type_string[instruction_fetched->instruction], t_register_string[(int)param1->parameter], t_register_string[(int)param2->parameter]);
					add_execute(pcb, (t_register)param1->parameter, (t_register)param2->parameter);
					pcb->program_counter++;

					break;
				case MOV_IN:

					log_info(logger, "PID: %i - Ejecutando: %s - %s - %i", pcb->id, t_instruction_type_string[instruction_fetched->instruction], t_register_string[(int)param1->parameter], (int)param2->parameter);
					mov_execute(pcb, (t_register)param1->parameter, (uint32_t)param2->parameter, IN);

					break;
				case MOV_OUT:

					log_info(logger, "PID: %i - Ejecutando: %s - %i - %s", pcb->id, t_instruction_type_string[instruction_fetched->instruction], (int)param1->parameter, t_register_string[(int)param2->parameter]);
					mov_execute(pcb, (t_register)param2->parameter, (uint32_t)param1->parameter, OUT);

					break;
				case IO:

					log_info(logger, "PID: %i - Ejecutando: %s - %s - %i", pcb->id, t_instruction_type_string[instruction_fetched->instruction], (char*)param1->parameter, (int)param2->parameter);
					device = (char*)param1->parameter;
					io_value = (int)param2->parameter;
					interruption_io_pf = INT_IO;
					pcb->program_counter++;

					break;
				case EXIT:
					// Guardar directo en pcb para hacer check al final
					log_info(logger, "PID: %i - Ejecutando: %s", pcb->id, t_instruction_type_string[instruction_fetched->instruction]);
					pcb->interrupt_type = EXECUTION_FINISHED;
					for(int i = list_size(tlb) - 1; i >= 0; i--){
						t_tlb* delete_tlb = list_get(tlb,i);
						if(delete_tlb->pid == pcb->id){
							delete_tlb = list_remove(tlb,i);
							free(delete_tlb);
						}
					}
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
				send_pcb_pf(socket_kernel_dispatch, pcb, pf_segment, pf_page);
				break;
			default:
				send_pcb(socket_kernel_dispatch, pcb);
				break;
		}
		log_pcb(logger, pcb);
		log_tlb(logger, tlb);
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
	usleep(instruction_delay * 1000);

}



void add_execute(t_pcb* pcb, t_register reg1, t_register reg2){

	pcb->registers[reg1] = pcb->registers[reg1]+pcb->registers[reg2];
	usleep(instruction_delay * 1000);

}



void mov_execute(t_pcb* pcb, t_register reg1, uint32_t dl, int in_out){
	// MMU
	int segment_max_size;
	int segment_num;
	int segment_offset;
	int page_num;
	int page_offset;
	mmu(dl, &segment_max_size, &segment_num, &segment_offset, &page_num, &page_offset);
	// Segmentation fault?
	t_segment* segment_pcb = (t_segment*)list_get(pcb->segment_table,segment_num);
	if(segment_offset >= segment_pcb->size){
		interruption_io_pf = SEGMENTATION_FAULT;
		log_trace(logger,"SEGMENTATION FAULT");
	}
	else{
		// Bloqueo
		pthread_mutex_lock(&sem_mutex_tlb);
		//Check TLB
		int frame = check_tlb(pcb->id, segment_num, page_num);
		//1 o 2 accesos
		if(frame != -1){
			log_info(logger, "PID: %i - TLB HIT - Segmento: %i - Pagina: %i", pcb->id, segment_num, page_num);
			tlb_access(pcb, segment_num, page_num, frame, page_offset, reg1, in_out);
		}
		else{
			log_info(logger, "PID: %i - TLB MISS - Segmento: %i - Pagina: %i", pcb->id, segment_num, page_num);
			// no esta el frame, hay que pedirlo
			send_frame_request(socket_memoria, pcb, segment_num, page_num);
			op_code op_code = recv_op_code(socket_memoria);
			if(op_code == FRAME_ACCESS){
				frame = recv_int(socket_memoria);
				log_trace(logger, "Frame Recibido: %i", frame);
				if(inputs_tlb > 0) 
				{
					// agrego a lista o reemplazo?
					if(list_size(tlb) < inputs_tlb){
						add_to_tlb(pcb->id, segment_num, page_num, frame);
						log_trace(logger, "Agrego entrada a la TLB");
					}
					else{
						replace_tlb_input(pcb->id, segment_num, page_num, frame);
						log_trace(logger, "Replace entrada en la TLB");
					}
					for(int i = 0; i < list_size(tlb); i++){
						t_tlb* aux_tlb = list_get(tlb,i);
						log_info(logger, "%i|PID:%i|SEGMENTO:%i|PAGINA:%i|MARCO:%i", i, aux_tlb->pid, aux_tlb->segment, aux_tlb->page, aux_tlb->frame);
					}
				}
				tlb_access(pcb, segment_num, page_num, frame, page_offset, reg1, in_out);
			}
			else if (op_code == PAGE_FAULT){
				pf_occurred(pcb->id, segment_num, page_num);
			}
			// Desbloqueo
			pthread_mutex_unlock(&sem_mutex_tlb);
		}
	}
	log_tlb(logger, tlb);
}


void mmu (int dl, int* segment_max_size, int* segment_num, int* segment_offset, int* page_num, int* page_offset){
	*segment_max_size = inputs_table_memory * page_size;
	*segment_num = floor(dl / *segment_max_size);
	*segment_offset = dl % *segment_max_size;
	*page_num = floor(*segment_offset / page_size);
	*page_offset = *segment_offset % page_size;
	log_trace(logger,"Segment max size: %i", *segment_max_size);
	log_trace(logger,"Segment num: %i", *segment_num);
	log_trace(logger,"Segment offset: %i", *segment_offset);
	log_trace(logger,"Page num: %i", *page_num);
	log_trace(logger,"Page offset: %i", *page_offset);
}



int check_tlb(int process_id, int segment_num, int page_num){     // Retorna marco si esta en TLB, -1 si no.
	t_tlb* input_tlb;	//declaro variables locales
	int frame = -1;
	for(int i = 0;i<list_size(tlb);i++){	// itero sobre entradas en tlb
		input_tlb = list_get(tlb,i);
		if(process_id == input_tlb->pid && segment_num == input_tlb->segment && page_num == input_tlb->page){
			frame = input_tlb->frame;
			if(strcmp(replace_tlb, "LRU") == 0){
				input_tlb->time = timestamp;
				timestamp++;
			}
		}
	}
	return frame;
}



void tlb_access(t_pcb* pcb, int segment_num, int page_num, int frame, int page_offset, uint32_t reg1, int in_out){
	// Acceder a memoria por el dato
	if(in_out == IN){
		request_data_in(frame, page_offset, pcb, reg1);
		log_info(logger, "PID: %i - Acción: LEER - Segmento: %i - Pagina: %i - Dirección Fisica: %i %i", pcb->id, segment_num, page_num, frame, page_offset);
	}
	else if(in_out == OUT){
		request_data_out(frame, page_offset, pcb, reg1);
		log_info(logger, "PID: %i - Acción: ESCRIBIR - Segmento: %i - Pagina: %i - Dirección Fisica: %i %i", pcb->id, segment_num, page_num, frame, page_offset);
	}
}



void add_to_tlb(int pid, int segment_num, int page_num, int frame){
	// creo tlb nueva
	t_tlb* new_tlb = malloc(sizeof(new_tlb));
	new_tlb->pid = pid;
	new_tlb->segment = segment_num;
	new_tlb->page = page_num;
	new_tlb->frame = frame;
	new_tlb->time = timestamp;
	timestamp++;
	list_add(tlb, new_tlb);
}



void replace_tlb_input(int pid, int segment_num, int page_num, int frame){
	// Itero y guardo indice de timestamp mas bajo
	t_tlb* aux_tlb = list_get(tlb,0);
	int replace_value = aux_tlb->time + 1; // creo valor ficticio
	int index_replace;					// indice de la victima
	for(int i = 0; i < list_size(tlb); i++){
		t_tlb* replaced_tlb = list_get(tlb,i);
		if(replaced_tlb->time < replace_value){
			replace_value = replaced_tlb->time;
			index_replace = i;
		}	// Se recorre la lista guardando siempre el menor time
	}		// al final, el index_replace marca el de menor time
	aux_tlb = list_get(tlb, index_replace);// reemplazo segun el index anterior
	aux_tlb->pid = pid;
	aux_tlb->segment = segment_num;
	aux_tlb->page = page_num;
	aux_tlb->frame = frame;
	aux_tlb->time = timestamp;
	timestamp++;
}



void request_data_in(int frame, int page_offset, t_pcb* pcb, t_register reg1){
	send_read_request(socket_memoria, pcb, frame, page_offset);
	recv_and_validate_op_code_is(socket_memoria, RAM_ACCESS_READ);
	pcb->registers[reg1] = recv_int(socket_memoria);
	pcb->program_counter++;
}


void request_data_out(int frame, int page_offset, t_pcb* pcb, t_register reg1){
	send_write_request(socket_memoria, pcb, frame, page_offset, pcb->registers[reg1]);
	recv_and_validate_op_code_is(socket_memoria, RAM_ACCESS_WRITE);
	pcb->program_counter++;
}



void pf_occurred(int pid, int segment_num, int page_num){
	interruption_io_pf = INT_PAGE_FAULT;
	pf_segment = segment_num;
	pf_page = page_num;
	log_info(logger, "Page Fault PID: %i - Segmento: %i - Pagina: %i", pid, segment_num, page_num);
}


void* consistency_check(void* arg) {
	// FUNCION PARA THREAD
	socket_memoria_tlb = start_client_module("MEMORIA_CPU_TLB");
	int frame_swapped;
	while(true){
		frame_swapped = recv_tlb_consistency_check(socket_memoria_tlb);
		// Bloqueo
		pthread_mutex_lock(&sem_mutex_tlb);
		for(int i = list_size(tlb) - 1; i >= 0; i--){
			t_tlb* delete_tlb = list_get(tlb,i);
			if(delete_tlb->frame == frame_swapped){
				delete_tlb = list_remove(tlb,i);
				free(delete_tlb);
			}
		}
		// Desbloqueo
		pthread_mutex_unlock(&sem_mutex_tlb);
	}
}

/*
   FALTA AGREGAR UN HILO QUE ESCUCHE LA MEMORIA Y SE ENCARGUE DE LIMPIAR LA TLB CON LAS COSAS
   QUE SE SWAPEEN PARA EVITAR INCONSISTENCIA, VA A HABER QUE SEMAFORIZAR LA TLB
*/


