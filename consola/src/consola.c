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
 2) ubicarse con una terminal en /consola/Debug
 3) ejecutar el siguiente comando: ./consola arg1 ../../config/instructions/program1.txt

 EXPLICACION DEL COMANDO:
 * Como primer argumento se pasa la direccion del archivo de config (todavia no se usa)
 * Como segundo argumento se para la direccion del archivo de instrucciones, cree un archivo
   que es igual al que dieron como ejemplo

 */

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <structures.h>
#include "parser.h"

t_log* logger;

void destroy_instruction(void* instruction) {
	list_destroy(((t_instruction*)instruction)->parameters);
}

int main(int argc, const char **argv) {

	logger = log_create("consola.log", "consola", true, LOG_LEVEL_INFO);

	// Obtengo las instrucciones
	t_list* instructions = parse(logger, argv[2]);

	// Las muestro por pantall (eliminar luego esto)
	for(int i = 0; i < list_size(instructions); i++) {
		t_instruction* inst = list_get(instructions, i);
		
		log_info(logger, "Instruction(%i, params: %i)", inst->operator, list_size(inst->parameters));
	}

	log_info(logger, "Cerrando consola...");

	// Hay que liberar la memoria de lo que se reservo
	list_destroy_and_destroy_elements(instructions, &destroy_instruction);
	log_destroy(logger);

	return EXIT_SUCCESS;
}
