/*
 ============================================================================
 Name        : kernel.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <shared/socket.h>
#include <shared/serialization.h>
#include <shared/structures.h>
#include <shared/log_extras.h>
#include <commons/log.h>

t_log* logger;

void destroy_instruction(void* instruction) {
	list_destroy(((t_instruction*)instruction)->parameters);
}

int main(void) {

	logger = log_create("kernel.log", "kernel", true, LOG_LEVEL_TRACE);

	int socket_server = start_server_module("KERNEL");
	log_trace(logger, "Esperando una conexion...");
	int socket_consola = accept(socket_server, NULL, NULL);

	char* msg = recv_string(socket_consola);
	log_trace(logger, "Se recibio un mensaje: %s", msg);

	t_list* instructions = recv_instructions(socket_consola);
	log_trace(logger, "Se recibieron instrucciones de consola");
	log_instructions(logger, instructions);

	t_pcb* pcb = malloc(sizeof(t_pcb));
	pcb->id = 99;
	pcb->process_size = 69;
	pcb->program_counter = 420;
	pcb->page_table = 1337;
	pcb->estimated_burst = 9.0;
	pcb->socket_consola = socket_consola;
	pcb->start_burst = 1.23;
	pcb->estimated_remaining_burst = 32.1;
	pcb->execution_time = 44.4;
	pcb->instructions = instructions;
	log_trace(logger, "Se genero la pcb");
	log_pcb(logger, pcb);

	send_pcb(socket_consola, pcb);
	log_trace(logger, "Se envio la pcb a consola");

	// Hay que liberar la memoria
	free(msg);
	free(pcb);
	list_destroy_and_destroy_elements(instructions, &destroy_instruction);
	log_destroy(logger);
}
