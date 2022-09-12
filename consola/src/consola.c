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

#include "parser.h"

t_log* logger;

void init() {
	logger = log_create("consola.log", "consola", true, LOG_LEVEL_INFO);
}
void free_resources() {
	log_destroy(logger);
}
void exit_error(const char* message) {
	log_error(logger, message);
	free_resources();
	exit(EXIT_FAILURE);
}
void parse_instructions(const char* path)
{
	if(path == NULL) {
		exit_error("Debe proveer el path para el archivo de instrucciones como segundo argumento\n");
	}

	FILE * instructions_stream = fopen(path, "r");

	log_info(logger, "Se abrio el archivo de instrucciones");

	if(instructions_stream == NULL) {
		exit_error("No se pudo abrir el archivo de instrucciones");
	}

	parse(logger, instructions_stream);
	
	fclose(instructions_stream);
}

int main(int argc, const char **argv) {

	init();

	parse_instructions(argv[2]);

	log_info(logger, "Cerrando consola...");

	free_resources();

	return EXIT_SUCCESS;
}
