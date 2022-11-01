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
//t_config* cpu_config;
//char* cpu_config_path;
//uint32_t inputs_tlb;
//char* replace_tlb;


int main(int argc, char **argv) {
	printf("aaaa");
	// Obtengo path de Config
	//cpu_config_path = string_new();
	//string_append(&cpu_config_path, argv[1]);
	// Inicio logger
	logger = log_create("cpu.log", "CPU", true, LOG_LEVEL_TRACE);
	// Creo config
	//cpu_config = config_create(cpu_config_path);
	// Error por si no se paso bien el argumento
	//if(cpu_config == NULL) {
	//	log_error(logger, "No se pudo abrir la config de CPU");
	//    exit(EXIT_FAILURE);
	//}
	// Obtengo datos de la config
	//replace_tlb = config_get_string_value(cpu_config, "REEMPLAZO_TLB");
	//log_trace(logger, "  REEMPLAZO_TLB: %s", replace_tlb);

	//inputs_tlb = config_get_int_value(cpu_config, "ENTRADAS_TLB");
	//log_trace(logger, "  ENTRADAS_TLB: %d", inputs_tlb);
	// CONECTAR A MEMORIA

	// CONECTAR A KERNEL
	//int socket_cpu = start_server_module("CPU");
	//log_trace(logger,"Esperando conexion con Kernel");
	//int socket_kernel = accept(socket_cpu, NULL, NULL);
	//log_trace(logger, "Conexion con kernel: %i", socket_kernel);
	// INICIAR INTERRUPT

	// INICIAR DISPATCH
	//t_pcb* pcb = recv_pcb(socket_kernel);


	//FREE MEM
	config_destroy(cpu_config);
	log_destroy(logger);
	return EXIT_SUCCESS;
}

