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
#include <commons/string.h>
#include <commons/collections/list.h>
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
// Config values
int max_degree_multiprogramming;
t_scheduling_algorithm scheduling_algorithm;
int quantum_rr;
t_list* io_devices_list;
t_list* io_times_list;

// Connections
int socket_cpu_interrupt;
int socket_cpu_dispatch;
int socket_memoria;

// Threads
pthread_t thread_schedulling;

int main(int argc, char **argv) 
{
	initialize_logger(argv);
	initialize_config(argv);
	initialize_sockets();
	initialize_scheduller();

	pthread_create(&thread_schedulling, NULL, start_schedulling, NULL); // thread schedulling

	int socket_kernel = start_server_module("KERNEL");
	while(true) {
		// Espero hasta que se conecte una consola
		log_trace(logger, "Esperando consola...");
		int socket_consola = accept(socket_kernel, NULL, NULL);
		log_trace(logger, "Conexion con consola: %i", socket_consola);
		t_list* instructions = recv_instructions(socket_consola);
		t_list* segments = recv_segments(socket_consola);
		create_process(socket_consola, instructions, segments);
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

    max_degree_multiprogramming = config_get_int_value(config, "GRADO_MAX_MULTIPROGRAMACION");

	char* shceduling_algorithm_string = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
	if(string_equals_ignore_case(shceduling_algorithm_string, "FIFO"))
		scheduling_algorithm = FIFO;
	else if(string_equals_ignore_case(shceduling_algorithm_string, "RR"))
		scheduling_algorithm = RR;
	else if(string_equals_ignore_case(shceduling_algorithm_string, "FEEDBACK"))
		scheduling_algorithm = FEEDBACK;

	quantum_rr = config_get_int_value(config, "QUANTUM_RR");
}

void initialize_sockets()
{
	socket_cpu_interrupt = start_client_module("CPU_INTERRUPT");
	socket_cpu_dispatch = start_client_module("CPU_DISPATCH");
	socket_memoria = start_client_module("MEMORIA_KERNEL");
}

int process_count = 0;
void create_process(int socket_consola, t_list* instructions, t_list* segments)
{
	// Comunicarse con la memoria y pedirle que genere las tablas de paginas para cada uno de los segmentos
	// Luego guardar todo en pcb->segment_table
	t_pcb* pcb = malloc(sizeof(t_pcb));
	pcb->id = ++process_count;
	pcb->interrupt_type = NO_INTERRUPT;
	pcb->socket_consola = socket_consola;
	pcb->instructions = instructions;
	pcb->registers[AX] = 0;
	pcb->registers[BX] = 0;
	pcb->registers[CX] = 0;
	pcb->registers[DX] = 0;

	int segments_count = list_size(segments);
    for(int i = 0; i < segments_count; i++)
    {
        int segment = list_get(segments, i);
		log_trace(logger, "Segment %i: %i", i, segment);
    }

	new_state(pcb);
}
