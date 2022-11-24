/*
 ============================================================================
 Name        : consola.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================

 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
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

t_list* get_segments_from_config()
{
	char** segments_temp = config_get_array_value(consola_config, "SEGMENTOS");
	t_list* segments = list_create();

	int i = 0;
	while(segments_temp[i] != NULL) {
		int segment = atoi(segments_temp[i]);
		list_add(segments, segment);
		log_trace(logger, "segment: %i", segment);
		i++;
	}

	return segments;
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
	// Obtengo los segmentos
	t_list* segments = get_segments_from_config();

	int socket_kernel = start_client_module("KERNEL");
	// Envio las instrucciones al kernel
	send_instructions(socket_kernel, instructions);
	// Envio los segmentos leidos por config
	send_segments(socket_kernel, segments);

	log_trace(logger, "Envie a kernel las instrucciones");
	log_instructions(logger, instructions);

	// Ciclo de espera con el Kernel
	while(true) {
		log_trace(logger, "Espero al Kernel...");
		op_code op = recv_op_code(socket_kernel);

		switch (op)
		{
		case TECLADO:
			// Espero un input por el teclado
			char input[256];
			do {
				printf("Escriba un numero: ");
				scanf("%s", input);
			}
			while(strlen(input) != strlen(string_itoa(atoi(input))));

			// Mando al Kernel lo que obtuve del teclado
			int teclado = atoi(input);
			send_teclado_response(socket_kernel, teclado);
			break;
		
		case PANTALLA: 
			// Recibo el valor a imprimir por pantalla
			int pantalla = recv_int(socket_kernel);
			printf("Resultado: %i\n", pantalla);
			// Le aviso al Kernel que ya mostre por pantalla el valor
			send_pantalla_response(socket_kernel);
			break;

		case SEG_FAULT:
			printf("Error: Segmentation Fault (SIGSEGV)\n");
			goto exit_cycle;
			break;

		case PROCESS_FINISHED:
		default:
			log_trace(logger, "Fianlizo la ejecucion de consola...");
			goto exit_cycle;
			break;
		}
	}
	exit_cycle: ;

	// Hay que liberar la memoria_config de lo que se reservo
	list_destroy_and_destroy_elements(instructions, &destroy_instruction);
	log_destroy(logger);
	config_destroy(consola_config);

	return EXIT_SUCCESS;
}

