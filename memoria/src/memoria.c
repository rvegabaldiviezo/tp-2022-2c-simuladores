/*
 ============================================================================
 Name        : memoria.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <shared/log_extras.h>
#include <shared/structures.h>
#include <shared/structures_translation.h>
#include <shared/serialization.h>
#include <shared/socket.h>
#include "memoria.h"
#include "handle_kernel.h"
#include "handle_cpu.h"

t_log* logger;
t_config* config;
// Config
t_memoria_config* memoria_config;

// Sockets
int socket_cpu;
int socket_kernel;

// Threads
pthread_t thread_cpu;

// Estructuras de la memoria
void* ram;
FILE* swap;
t_dictionary* page_tables_per_pid;
// Key: PID, Value: lista de Tablas de Pagina
// Key: Segmento, Value: Tabla de Pagina
// Key: Pagina, Value: Marco

int main(int argc, char **argv) 
{
	initialize_logger(argv);
	initialize_config(argv);
	initialize_sockets();
	initialize_memory_structures();

	// Thread para manejar la comunicacion con cpu
	pthread_create(&thread_cpu, NULL, handle_cpu, NULL);
	// Manejo el kernel en el main thread
	handle_kernel();

	// Destruyo todo todito
	free(memoria_config);
	config_destroy(config);
	log_destroy(logger);

	return EXIT_SUCCESS;
}

void initialize_logger(char **argv)
{
	logger = log_create("memoria.log", "memoria", true, LOG_LEVEL_TRACE);
}

void initialize_sockets()
{
	// Conexion de Servidor con CPU
	int socket_memoria_cpu = start_server_module("MEMORIA_CPU");
	log_trace(logger,"Esperando conexion con CPU desde MEMORIA");
	socket_cpu = accept(socket_memoria_cpu, NULL, NULL);
	log_trace(logger, "Conexion con cpu: %i", socket_cpu);

	// Conexion de Servidor con Kernel
	int socket_memoria_kernel = start_server_module("MEMORIA_KERNEL");
	log_trace(logger,"Esperando conexion con KERNEL desde MEMORIA");
	socket_kernel = accept(socket_memoria_kernel, NULL, NULL);
	log_trace(logger, "Conexion con kernel: %i", socket_kernel);
}

void initialize_config(char **argv)
{
	// Config Path
	char* memoria_config_path = argv[1];

	config = config_create(memoria_config_path);

	if(config == NULL) {
		log_error(logger, "No se pudo abrir la config de memoria");
		exit(EXIT_FAILURE);
	}

	// Datos de config
	memoria_config = (t_memoria_config*)malloc(sizeof(t_memoria_config));

	memoria_config->memory_size = config_get_int_value(config, "TAM_MEMORIA");
	memoria_config->page_size = config_get_int_value(config, "TAM_PAGINA");
	memoria_config->inputs_table = config_get_int_value(config, "ENTRADAS_POR_TABLA");
	memoria_config->memory_delay = config_get_int_value(config, "RETARDO_MEMORIA");
	memoria_config->replace_algorithm = config_get_string_value(config, "ALGORITMO_REEMPLAZO");
	memoria_config->frames_per_process = config_get_int_value(config, "MARCOS_POR_PROCESO");
	memoria_config->swap_delay = config_get_int_value(config, "RETARDO_SWAP");
	memoria_config->path_swap = config_get_string_value(config, "PATH_SWAP");
	memoria_config->swap_size = config_get_int_value(config, "TAMANIO_SWAP");
}

void initialize_memory_structures()
{
	ram = malloc(memoria_config->memory_size);
	swap = fopen(memoria_config->path_swap, "w+");
	ftruncate(memoria_config->path_swap, memoria_config->swap_size);
	page_tables_per_pid = dictionary_create();
}

