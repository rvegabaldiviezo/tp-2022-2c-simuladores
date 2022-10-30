/*
 ============================================================================
 Name        : consola.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================

 PARA EJECUTAR Y PROBAR LA CONSOLA:
 * ejecutar en la raiz del proyecto:
bash run.sh consola ../../config/base/consola.config ../../config/base/program1.txt

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
#include <shared/serialization.h>
#include <shared/socket.h>
#include <shared/environment_variables.h>
#include <shared/log_extras.h>
#include "parser.h"

t_log* logger;
t_config* consola_config;

void destroy_instruction(void* instruction) {
	list_destroy(((t_instruction*)instruction)->parameters);
}

int main(int argc, char **argv) {

	char* consola_config_path = argv[1];
	char* program_path = argv[2];

	logger = log_create("consola.log", "consola", true, LOG_LEVEL_TRACE);
	// Obtengo la config de consola
	consola_config = config_create(consola_config_path);

	if(consola_config == NULL) {
		log_error(logger, "No se pudo abrir la config de consola");
        exit(EXIT_FAILURE);
	}

	// Obtengo las instrucciones
	t_list* instructions = parse(program_path);

	int socket_kernel = start_client_module("KERNEL");
	// Envio las instrucciones al kernel
	send_instructions(socket_kernel, instructions);

	log_trace(logger, "Envie a kernel las instrucciones");
	log_instructions(logger, instructions);

	// Hay que liberar la memoria de lo que se reservo
	list_destroy_and_destroy_elements(instructions, &destroy_instruction);
	log_destroy(logger);
	config_destroy(consola_config);

	return EXIT_SUCCESS;
}

