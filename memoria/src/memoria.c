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
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <commons/bitarray.h>
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
int socket_cpu_tlb;
int socket_kernel;
int socket_kernel_page_fault;

// Threads
pthread_t thread_cpu;

// Estructuras de la memoria
void* ram;
FILE* swap;
t_list* page_tables;
t_list* frames_usage; // Frames usados 0 -> libre, 1 -> ocupado
t_list* last_page_table_reference;
int time;

// Mutex
pthread_mutex_t ram_mutex;

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
	logger = log_create("memoria.log", "memoria", true, LOG_LEVEL_INFO);
}

void initialize_sockets()
{
	// Conexion de Servidor con CPU
	int socket_memoria_cpu = start_server_module("MEMORIA_CPU");
	log_trace(logger,"Esperando conexion con CPU desde MEMORIA");
	socket_cpu = accept(socket_memoria_cpu, NULL, NULL);
	log_trace(logger, "Conexion con cpu: %i", socket_cpu);

	// Conexion de Servidor con CPU TLB
	int socket_memoria_cpu_tlb = start_server_module("MEMORIA_CPU_TLB");
	log_trace(logger,"Esperando conexion con CPU TLB desde MEMORIA");
	socket_cpu_tlb = accept(socket_memoria_cpu_tlb, NULL, NULL);
	log_trace(logger, "Conexion con cpu tlb: %i", socket_cpu_tlb);

	// Conexion de Servidor con Kernel
	int socket_memoria_kernel = start_server_module("MEMORIA_KERNEL");
	log_trace(logger,"Esperando conexion con KERNEL desde MEMORIA");
	socket_kernel = accept(socket_memoria_kernel, NULL, NULL);
	log_trace(logger, "Conexion con kernel: %i", socket_kernel);

	// Conexion de Servidor con Kernel
	int socket_memoria_kernel_page_fault = start_server_module("MEMORIA_KERNEL_PAGE_FAULT");
	log_trace(logger,"Esperando conexion con KERNEL PAGE FAULT desde MEMORIA");
	socket_kernel_page_fault = accept(socket_memoria_kernel_page_fault, NULL, NULL);
	log_trace(logger, "Conexion con kernel: %i", socket_kernel_page_fault);
	log_trace(logger, "Se inicializaron los sockets de la memoria correctamente");
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
	log_trace(logger, "Se inicializo la config de la memoria correctamente");
}

void initialize_memory_structures()
{
	ram = malloc(sizeof(int) * memoria_config->memory_size);
	log_trace(logger, "Se inicializo la ram");
	swap = fopen(memoria_config->path_swap, "w+");
	fclose(swap);
	log_trace(logger, "Se inicializo archivo swap");
	ftruncate(memoria_config->path_swap, sizeof(int) * memoria_config->swap_size);
	log_trace(logger, "Se modifico el tamaño del archivo swap");
	page_tables = list_create();
	time = 0;
	last_page_table_reference = list_create();

	int frames_count = memoria_config->frames_per_process;
	frames_usage = list_create();

	for(int i = 0; i < frames_count; i++)
	{
		list_add(frames_usage, false);
	}

	log_trace(logger, "Cantidad de frames: %i", list_size(frames_usage));
	pthread_mutex_init(&ram_mutex, NULL);
	log_trace(logger, "Se inicializaron las estructuras de la memoria correctamente");
}

t_page_table_data* get_page(t_pcb* pcb, int segment, int page)
{
	t_segment* segment_data = list_get(pcb->segment_table, segment);
    t_list* page_table = list_get(page_tables, segment_data->page_table_index);
    return list_get(page_table, page);
}


// parametros pasados por cpu para encontrar la pagina a travez del frame
t_page_table_data* get_page_reverse(t_pcb* pcb, int frame)
{
	// hacemos un for para buscar los segmentos 
	for(int segment = 0; segment < list_size(pcb->segment_table); segment++)
	{
		t_segment* segment_data = list_get(pcb->segment_table, segment);
		t_list* page_table = list_get(page_tables, segment_data->page_table_index);
		// hacemos otro for para buscar en la tabla de paginas
		for(int page = 0; page < list_size(page_table); page++)
		{
			t_page_table_data* page_data = list_get(page_table, page);

			if(page_data->P == 1 && page_data->frame == frame)
				return page_data;
		}
	}
}
