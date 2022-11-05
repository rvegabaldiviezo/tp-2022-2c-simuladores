#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdint.h>
#include <commons/log.h>
#include <commons/config.h>
#include <shared/log_extras.h>
#include <shared/structures.h>
#include <shared/serialization.h>
#include <shared/socket.h>
#include <shared/environment_variables.h>
#include <pthread.h>
#include <semaphore.h>
#include "cpu.h"

t_log* logger;
t_config* cpu_config;
uint32_t inputs_tlb;
int instruction_delay,
	interruption_quantum,
	interruption_io_pf,
	socket_cpu_dispatch,
	socket_kernel_dispatch,
	instruction_delay_sec;
char* replace_tlb;
pthread_t thread_interrupt;
//sem_t sem_interrupt;


int main(int argc, char **argv) {

	// Logger

	logger = log_create("cpu.log", "CPU", true, LOG_LEVEL_TRACE);

	// Obtengo path de Config

	char* cpu_config_path = argv[1];

	// Creo config

	cpu_config = config_create(cpu_config_path);

	// Obtengo datos de la config

	replace_tlb = config_get_string_value(cpu_config, "REEMPLAZO_TLB");
	log_trace(logger, "  REEMPLAZO_TLB: %s", replace_tlb);

	inputs_tlb = config_get_int_value(cpu_config, "ENTRADAS_TLB");
	log_trace(logger, "  ENTRADAS_TLB: %d", inputs_tlb);

	instruction_delay = config_get_int_value(cpu_config, "RETARDO_INSTRUCCION");
	log_trace(logger, "  RETARDO_INSTRUCCION: %i ms", instruction_delay);
	instruction_delay_sec = instruction_delay * 0.001; // para poder utilizar la variable dentro de sleep

	// Error por si no se paso bien el argumento

	if(cpu_config == NULL) {
		log_error(logger, "No se pudo abrir la config de CPU");
		exit(EXIT_FAILURE);
	}

	connections();

	instruction_cycle();

	free_memory();

	// EXIT
	return EXIT_SUCCESS;
}



void* start_interrupt(void* arg) {
	// FUNCION PARA THREAD

	int socket_cpu_interrupt = start_server_module("CPU_INTERRUPT");
	log_trace(logger,"Esperando conexion con Kernel desde INTERRUPT");
	int socket_kernel_interrupt = accept(socket_cpu_interrupt, NULL, NULL);
	log_trace(logger, "Conexion con kernel interrupt: %i", socket_kernel_interrupt);

	/*while(true){
		// Introducir recibir interrupcion   EXECUTION_FINISHED > INT_IO = PAGE_FAULT > INT_QUANTUM FIJARSE CONDICIONES CARRERA
		interruption_quantum = INT_QUANTUM;
		sem_wait(sem_interrupt);
		interruption_quantum = NO_INTERRUPT;
	}*/
}





void connections(){

	// CONECTAR A MEMORIA

	// CONECTAR A KERNEL

	// INTERRUPT

	pthread_create(&thread_interrupt, NULL, start_interrupt, NULL); // thread interrupt

	// DISPATCH

	socket_cpu_dispatch = start_server_module("CPU_DISPATCH");
	log_trace(logger,"Esperando conexion con Kernel desde DISPATCH");
	socket_kernel_dispatch = accept(socket_cpu_dispatch, NULL, NULL);

	// QUEDA BLOQUEADO HASTA TENER CONEXIÓN CON KERNEL_DISPATCH

	log_trace(logger, "Conexion con kernel dispatch: %i", socket_kernel_dispatch);

}



void instruction_cycle(){

	while(true){
		t_pcb* pcb = recv_pcb(socket_kernel_dispatch);
		pcb->interrupt_type = NO_INTERRUPT;
		interruption_quantum = NO_INTERRUPT;
		interruption_io_pf = NO_INTERRUPT;
		log_trace(logger, "Recibi PCB y ciclo instruccion");
		while(pcb->interrupt_type == NO_INTERRUPT){
			// FETCH

			t_instruction* instruction_fetched = list_get(pcb->instructions,pcb->program_counter);

			//log_instructions(logger, pcb->instructions);

			log_trace(logger, "FETCH COMPLETO");
			// DECODE & EXECUTE

			t_parameter* param1;
			t_parameter* param2;

			if(instruction_fetched->instruction != EXIT){

				param1 = (t_parameter*)list_get(instruction_fetched->parameters,0);
				param2 = (t_parameter*)list_get(instruction_fetched->parameters,1);

			}
			switch(instruction_fetched->instruction){
				case SET:

					set_execute(pcb, (t_register)param1->parameter, (uint32_t)param2->parameter);

					break;
				case ADD:

					add_execute(pcb, (t_register)param1->parameter, (t_register)param2->parameter);

					break;
				case MOV_IN:

					//t_register reg1 = list_get(instruction_fetched->parameters,0);
					//uint32_t param1 = list_get(instruction_fetched->parameters,1);
					//mov_in_execute(pcb, reg1, param1);

					break;
				case MOV_OUT:

					//uint32_t param1 = list_get(instruction_fetched->parameters,0);
					//t_register reg1 = list_get(instruction_fetched->parameters,1);

					break;
				case IO:



					break;
				case EXIT:
					// Guardar directo en pcb para hacer check al final
					pcb->interrupt_type = EXECUTION_FINISHED;

					break;
			}
			log_trace(logger, "DECODE/EXECUTE COMPLETO");
			pcb->program_counter++;

			// CHECK INTERRUPT

			if(pcb->interrupt_type != EXECUTION_FINISHED){
				if(interruption_io_pf == NO_INTERRUPT){
					pcb->interrupt_type = interruption_quantum;
				}
				else{
					pcb->interrupt_type = interruption_io_pf;
				}
			}
			log_trace(logger, "CHECK INTERRUPT");
			//log_trace(logger, "AX: %i", pcb->registers[AX]);
			//log_trace(logger, "BX: %i", pcb->registers[BX]);
			//log_trace(logger, "CX: %i", pcb->registers[CX]);
			//log_trace(logger, "DX: %i", pcb->registers[DX]);
		}
		log_pcb(logger, pcb);
		send_pcb(socket_kernel_dispatch, pcb);
		//sem_post(sem_interrupt);
		log_trace(logger, "Envio PCB modificado");

	}
}



void free_memory(){

	config_destroy(cpu_config);
	log_destroy(logger);

}



void set_execute(t_pcb* pcb, t_register reg1, uint32_t param1){

	pcb->registers[reg1] = param1;
	sleep(instruction_delay_sec);

}

void add_execute(t_pcb* pcb, t_register reg1, t_register reg2){

	pcb->registers[reg1] = pcb->registers[reg1]+pcb->registers[reg2];
	sleep(instruction_delay_sec);

}

/*void mov_in_execute(t_pcb* pcb, t_register reg1, uint32_t param1){




}*/
