#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <shared/structures.h>
#include <string.h>
#include "parser.h"

// Defino estructuras estaticas que ayudan a relacionar
// un enum con un string
const static struct {
    const char *str;
    t_operator op;
} t_operator_conversion [] = {
    {"SET", SET},
    {"ADD", ADD},
    {"MOV_IN", MOV_IN},
    {"MOV_OUT", MOV_OUT},
    {"I/O", IO},
    {"EXIT", EXIT}
};
const static struct {
    const char *str;
    t_parameter param;
} t_parameter_conversion [] = {
    {"AX", AX},
    {"BX", BX},
    {"CX", CX},
    {"DX", DX},
    {"DISCO", DISCO}
};


// Dado un string lo transforma en su ENUM de t_operation correspondiente
// si no encuentra el ENUM devuelve -1
t_operator str2op(char* str)
{
    int j;
    for (j = 0;  j < sizeof (t_operator_conversion) / sizeof (t_operator_conversion[0]);  ++j)
        if (!strcmp (str, t_operator_conversion[j].str))
            return t_operator_conversion[j].op;
    return -1;
}
// Dado un string lo transforma en su ENUM de t_parameter correspondiente
// si no encuentra el ENUM devuelve -1
t_parameter str2param(char* str)
{
    int j;
    for (j = 0;  j < sizeof (t_parameter_conversion) / sizeof (t_parameter_conversion[0]);  ++j)
        if (!strcmp (str, t_parameter_conversion[j].str))
            return t_parameter_conversion[j].param;
    return -1;
}


// Devuelve la siguiente palabra de un stream
//
// IMPORTANTE: Debe liberarse la memoria de la palabra devuelta
char* next_word(FILE* stream)
{
    // defino que mi tamaño maximo de palabra sea 10 caracters
    const int word_size = 10;
    char c;
    int i = 0;
    // reservo la memoria para una palabra
    // es por esto que el que llama esta funcion tiene la obligacion
    // de liberarla
    char* word = malloc(sizeof (char) * word_size);

    // Saltearse todo el espacio en blanco de que esta adelante de una palabra.
    // POR EJEMPLO: "   \n   \t palabra" => "palabra"
    while((c = getc(stream)) == ' ' || c == '\t' || c == '\n');

    
    do {
        if(i > word_size) {
            // La palabra que estoy acumulando se volvio mas grande que mi limite
            // EXPLOTO
            exit(EXIT_FAILURE);
        }
        // acumulo dentro de mi memoria la palabra caracter por caracter
        word[i++] = c;
    }
    // itero por cada caracter hasta que me encuentre con un espacio, 
    // nueva linea o con el final del stream
    while((c = getc(stream)) != ' ' && c != '\t' && c != '\n' && c != EOF);
    // meto el ultimo caracter que saque nuevamente en el stream
    // esto es un HACK
    ungetc(c, stream);

    // para que nuestra palabra este correctamente formada debe tener
    // un caracter terminante al final.
    word[i] = '\0';

    return word;
}


// Devuelve true si el proximo caracter
// de un stream es el newline '\n'
bool next_is_newline(FILE* stream)
{
    char c = getc(stream);
    ungetc(c, stream);
    return c == '\n';
}
// Devuelve true si estoy en el final del stream
bool next_is_eof(FILE* stream)
{
    char c = getc(stream);
    ungetc(c, stream);
    return c == EOF;
}

// Abre un archivo dado un path, devuelve el stream del archivo
FILE* open_stream(t_log* logger, const char* path)
{
	if(path == NULL) {
        log_error(logger, "Debe proveer el path para el archivo de instrucciones como segundo argumento");
		exit(EXIT_FAILURE);
	}

	FILE * instructions_stream = fopen(path, "r");

	log_info(logger, "Se abrio el archivo de instrucciones");

	if(instructions_stream == NULL) {
		log_error(logger, "No se pudo abrir el archivo de instrucciones");
        exit(EXIT_FAILURE);
	}

    return instructions_stream;
}

// Crea una lista de instrucciones dada la direccion de un archivo
// 
// IMPORTANTE: Hay que liberar la memoria de cada instruccion
t_list* parse(t_log* logger, const char* path)
{
    // abro el archivo
    FILE* stream = open_stream(logger, path);

    // inicializo el listado de instrucciones que voy a devolver
    t_list* instructions = list_create();

    // mientras no este en el final del archivo sigo leyendo
    while(!next_is_eof(stream))
    {
        t_instruction* instruction = malloc(sizeof(t_instruction));
        instruction->parameters = list_create();

        // la primer palabra que obtengo deberia ser siempre un operador
        char* word = next_word(stream);
        t_operator operator = str2op(word);
        if(operator == -1) {
            // Is not an operator
            log_error(logger, "Se esperaba un operador, pero se encontro: %s", word);
            exit(EXIT_FAILURE);
        }
        instruction->operator = operator;
        // no me olvido de liberar la memoria de lo que ya no uso
        free(word);

        // las siguientes palabras entonces deben ser los parametros
        // reviso una por una las palabras hasta que me encuentro con
        // una nueva linea O el fin del archivo
        while(!next_is_newline(stream) && !next_is_eof(stream)) {
            word = next_word(stream);
            t_parameter parameter = str2param(word);
            // un parametro puede ser alguno de los enum definidos o puede ser un
            // numero cualquiera
            if(parameter == -1) {
                // la instruccion es del tipo "1" "2" "3" etc
            	list_add(instruction->parameters, (void*)atoi(word));
            }
            else {
                // la instruccion es del tipo "AX" "DISCO" "BX" etc
                list_add(instruction->parameters, (void*)parameter);
            }
            // no me olvido de liberar la memoria de lo que ya no uso
            free(word);
        }

        // ya tengo mi instruccion que esta formada por un operador y multiples parametros
        list_add(instructions, instruction);
    }

	log_info(logger, "Se cierra el archivo de instrucciones");
    // cerramos el archivo por su puesto
    fclose(stream);

    return instructions;
}
