#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <commons/log.h>
#include <commons/config.h>
#include <shared/structures.h>
#include <shared/serialization.h>
#include <shared/socket.h>
#include <shared/environment_variables.h>
#include <pthread.h>
#include "cpu.h"

t_log* logger;
t_config* cpu_config;
uint32_t inputs_tlb;
char* replace_tlb;
pthread_t thread_interrupt;


int main(int argc, char **argv) {
	// Obtengo path de Config
	char* cpu_config_path = argv[1];
	// Inicio logger
	logger = log_create("cpu.log", "CPU", true, LOG_LEVEL_TRACE);
	// Creo config
	cpu_config = config_create(cpu_config_path);
	// Error por si no se paso bien el argumento
	if(cpu_config == NULL) {
		log_error(logger, "No se pudo abrir la config de CPU");
		exit(EXIT_FAILURE);
	}
	// Obtengo datos de la config
	replace_tlb = config_get_string_value(cpu_config, "REEMPLAZO_TLB");
	log_trace(logger, "  REEMPLAZO_TLB: %s", replace_tlb);

	inputs_tlb = config_get_int_value(cpu_config, "ENTRADAS_TLB");
	log_trace(logger, "  ENTRADAS_TLB: %d", inputs_tlb);
	// CONECTAR A MEMORIA

	// CONECTAR A KERNEL
	pthread_create(&thread_interrupt, NULL, start_interrupt, NULL);
	int socket_cpu_dispatch = start_server_module("CPU_DISPATCH");
	log_trace(logger,"Esperando conexion con Kernel desde DISPATCH");
	int socket_kernel_dispatch = accept(socket_cpu_dispatch, NULL, NULL);
	log_trace(logger, "Conexion con kernel: %i", socket_kernel_dispatch);
	//t_pcb* pcb = recv_pcb(socket_kernel);


	//FREE MEM
	config_destroy(cpu_config);
	log_destroy(logger);
	return EXIT_SUCCESS;
}

void* start_interrupt(void* arg) {

	int socket_cpu_interrupt = start_server_module("CPU_INTERRUPT");
	log_trace(logger,"Esperando conexion con Kernel desde INTERRUPT");
	int socket_kernel_interrupt = accept(socket_cpu_interrupt, NULL, NULL);
	log_trace(logger, "Conexion con kernel: %i", socket_kernel_interrupt);

}
