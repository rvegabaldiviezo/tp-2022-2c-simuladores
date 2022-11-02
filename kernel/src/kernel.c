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
#include <pthread.h>
#include <unistd.h>
#include <commons/log.h>
#include <commons/config.h>
#include <shared/socket.h>
#include <shared/serialization.h>
#include <shared/structures.h>
#include <shared/log_extras.h>
#include "kernel.h"
#include "scheduller.h"

// Logger
t_log* logger;

// Config
t_config* config;

// Connections
int socket_cpu_interrupt;
int socket_cpu_dispatch;
int socket_memoria;

int main(int argc, char **argv) 
{
	initialize_logger(argv);
	initialize_config(argv);
	initialize_sockets();
	initialize_queues();

	int socket_kernel = start_server_module("KERNEL");
	while(true) {
		// Espero hasta que se conecte una consola
		log_trace(logger, "Esperando consola...");
		int socket_consola = accept(socket_kernel, NULL, NULL);
		log_trace(logger, "Conexion con consola: %i", socket_consola);
		t_list* instructions = recv_instructions(socket_consola);
		create_process(socket_consola, instructions);
	}
}

void initialize_logger(char **argv)
{
	logger = log_create("kernel.log", "kernel", true, LOG_LEVEL_TRACE);
}

void initialize_config(char **argv)
{
	config = config_create(argv[1]);
	if(config == NULL) {
		log_error(logger, "No se pudo abrir la config de kernel");
        exit(EXIT_FAILURE);
	}
}

void initialize_sockets()
{
	socket_cpu_interrupt = start_client_module("CPU_INTERRUPT");
	socket_cpu_dispatch = start_client_module("CPU_DISPATCH");
	//socket_memoria = start_client_module("MEMORIA");
}

int process_count = 0;
void create_process(int socket_consola, t_list* instructions)
{
	t_pcb* pcb = malloc(sizeof(t_pcb));
	pcb->id = ++process_count;
	pcb->interrupt_type = -1;
	pcb->socket_consola = socket_consola;
	pcb->instructions = instructions;

	new_state(pcb);
	execute_algorithm();
	wait_cpu_dispatch();
}
