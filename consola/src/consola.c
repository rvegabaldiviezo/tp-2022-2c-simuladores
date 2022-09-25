/*
 ============================================================================
 Name        : consola.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================

 PARA EJECUTAR Y PROBAR LA CONSOLA:
 1) buildear el proyecto de consola
 2) ubicarse con una terminal: cd ./consola/Debug
 3) ejecutar el siguiente comando: ./consola arg1 ../../config/base/program1.txt

 EXPLICACION DEL COMANDO:
 * Como primer argumento se pasa la direccion del archivo de config (todavia no se usa)
 * Como segundo argumento se para la direccion del archivo de instrucciones, cree un archivo
   que es igual al que dieron como ejemplo

 */

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/config.h>
#include <shared/structures.h>
#include <shared/connection.h>
#include <shared/environment_variables.h>
#include "parser.h"

t_log* logger;
t_config* consola_config;
t_config* ip_config;

void destroy_instruction(void* instruction) {
	list_destroy(((t_instruction*)instruction)->parameters);
}

int main(int argc, const char **argv) {

	char* consola_config_path = argv[1];
	char* program_path = argv[2];

	logger = log_create("consola.log", "consola", true, LOG_LEVEL_INFO);

	int socket = start_client("0.0.0.0", "8000");
	send_msg("Hola", socket);


	//ip_config = config_create(IP_CONFIG_PATH);
	//consola_config = config_create(consola_config_path);

	// Obtengo las instrucciones
	t_list* instructions = parse(logger, program_path);

	// Las muestro por pantall (eliminar luego esto)
	for(int i = 0; i < list_size(instructions); i++) {
		t_instruction* inst = list_get(instructions, i);
		
		log_info(logger, "Instruction %i", inst->operation);

		t_list* parameters = inst->parameters;

		for(int j = 0; j < list_size(parameters); j++)
		{
			void* param = list_get(parameters, j);
			log_info(logger, "\tparam: %i", (int)param);
		}
	}

	log_info(logger, "Cerrando consola...");

	// Hay que liberar la memoria de lo que se reservo
	list_destroy_and_destroy_elements(instructions, &destroy_instruction);
	log_destroy(logger);
	//config_destroy(ip_config);
	//config_destroy(consola_config);

	return EXIT_SUCCESS;
}
