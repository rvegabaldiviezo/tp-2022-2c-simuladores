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
#include <commons/log.h>

t_log* logger;

void destroy_instruction(void* instruction) {
	list_destroy(((t_instruction*)instruction)->parameters);
}

void log_instructions(t_list* instructions)
{
	// Las muestro por pantall (eliminar luego esto)
	for(int i = 0; i < list_size(instructions); i++) {
		t_instruction* inst = list_get(instructions, i);

		log_trace(logger, "Instruction %i", inst->instruction);

		t_list* parameters = inst->parameters;

		for(int j = 0; j < list_size(parameters); j++)
		{
			t_parameter* param = (t_parameter*)list_get(parameters, j);

			if(param->is_string)
				log_trace(logger, "\tparam: %s", (char*)param->parameter);
			else
				log_trace(logger, "\tparam: %i", (int)param->parameter);
		}
	}
}

int main(void) {

	logger = log_create("kernel.log", "kernel", true, LOG_LEVEL_TRACE);

	int socket_server = start_server_module("KERNEL");
	log_trace(logger, "Esperando una conexion...");
	int socket_client = accept(socket_server, NULL, NULL);

	char* msg = recv_string(socket_client);
	log_trace(logger, "Se recibio un mensaje: %s", msg);

	t_list* instructions = recv_instructions(socket_client);
	log_trace(logger, "Se recibieron instrucciones de consola");
	log_instructions(instructions);

	// Hay que liberar la memoria
	free(msg);
	list_destroy_and_destroy_elements(instructions, &destroy_instruction);
	log_destroy(logger);
}
