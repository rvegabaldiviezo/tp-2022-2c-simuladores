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
#include <commons/log.h>
#include <commons/config.h>
#include <shared/log_extras.h>
#include <shared/structures.h>
#include <shared/structures_translation.h>
#include <shared/serialization.h>
#include <shared/socket.h>
#include <shared/environment_variables.h>
#include <pthread.h>
#include <semaphore.h>

t_log* logger;
t_config* memory_config;

int memory_size,
	page_size,
	inputs_table,
	memory_delay,
	frames_per_process,
	swap_delay,
	swap_size,
	socket_memory_cpu,
	socket_cpu,
	socket_memory_kernel,
	socket_kernel;
char* replace_algorithm;
char* path_swap;

int main(int argc, char **argv) {

	// Logger

	logger = log_create("memoria.log", "CPU", true, LOG_LEVEL_TRACE);

	// Config Path

	char* memory_config_path = argv[1];
	log_trace(logger,"%s",memory_config_path);
	// Config

	memory_config = config_create(memory_config_path);
	log_trace(logger,"asd");

	if(memory_config == NULL) {
			log_error(logger, "No se pudo abrir la config de CPU");
			exit(EXIT_FAILURE);
	}

	// Datos de config

	memory_size = config_get_int_value(memory_config, "TAM_MEMORIA");
	log_trace(logger, "TAMANIO_MEMORIA: %i", memory_size);

	page_size = config_get_int_value(memory_config, "TAM_PAGINA");
	log_trace(logger, "TAMANIO_PAGINA: %i", page_size);

	inputs_table = config_get_int_value(memory_config, "ENTRADAS_POR_TABLA");
	log_trace(logger, "ENTRADAS_POR_TABLA: %i", inputs_table);

	memory_delay = config_get_int_value(memory_config, "RETARDO_MEMORIA");
	log_trace(logger, "RETARDO_MEMORIA: %i", memory_delay);

	replace_algorithm = config_get_string_value(memory_config, "ALGORITMO_REEMPLAZO");
	log_trace(logger, "ALGORITMO_REEMPLAZO: %s", replace_algorithm);

	frames_per_process = config_get_int_value(memory_config, "MARCOS_POR_PROCESO");
	log_trace(logger, "MARCOS_POR_PROCESO: %i", frames_per_process);

	swap_delay = config_get_int_value(memory_config, "RETARDO_SWAP");
	log_trace(logger, "RETARDO_SWAP: %i", swap_delay);

	path_swap = config_get_string_value(memory_config, "PATH_SWAP");
	log_trace(logger, "PATH_SWAP: %s", path_swap);

	swap_size = config_get_int_value(memory_config, "TAMANIO_SWAP");
	log_trace(logger, "TAMANIO_SWAP: %i", swap_size);

	// Conexion de Servidor con CPU

	socket_memory_cpu = start_server_module("MEMORIA_CPU");
	log_trace(logger,"Esperando conexion con CPU desde MEMORIA");
	socket_cpu = accept(socket_memory_cpu, NULL, NULL);
	log_trace(logger, "Conexion con kernel interrupt: %i", socket_cpu);

	// Conexion de Servidor con Kernel

	socket_memory_kernel = start_server_module("MEMORIA_KERNEL");
	log_trace(logger,"Esperando conexion con KERNEL desde MEMORIA");
	socket_kernel = accept(socket_memory_kernel, NULL, NULL);
	log_trace(logger, "Conexion con kernel interrupt: %i", socket_kernel);

	// Inserte programa aqui





	// Destruyo todo todito

	config_destroy(memory_config);
	log_destroy(logger);

	return EXIT_SUCCESS;
}

