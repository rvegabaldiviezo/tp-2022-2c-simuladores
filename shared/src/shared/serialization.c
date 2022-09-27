#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <commons/string.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include "serialization.h"
#include "structures.h"

extern t_log* logger;

// Envia a travez del socket un string
void send_string(int socket, char* string)
{
    int length = string_length(string) + 1;
    send(socket, &length, sizeof(int), 0);
    send(socket, string, sizeof(char) * length, 0);
}
// Recibe un string por el socket
// Se hace un malloc, por lo tanto no olvidarse de llamar FREE
// cuando no se use
char* recv_string(int socket)
{
    int length;
    recv(socket, &length, sizeof(int), 0);

    char* string = malloc(sizeof(char) * length);
    recv(socket, string, sizeof(char) * length, 0);

    return string;
}
// Se envia una lista de instrucciones a traves de un socket
void send_instructions(int socket, t_list* instructions)
{
    op_code op_code = INSTRUCTIONS;
    int instructions_count = list_size(instructions);

    // Envio el codigo de operacion
    send(socket, &op_code, sizeof(op_code), 0);
    // Envio un numero que representa cuantas instrucciones se mandan
    send(socket, &instructions_count, sizeof(int), 0);

    for(int i = 0; i < instructions_count; i++)
    {
        t_instruction* inst = list_get(instructions, i);

        t_instruction_type instruction_type = inst->instruction;
        int parameters_count = list_size(inst->parameters);

        // Envio el tipo de instruccion (SET, ADD, etc)
        // Y la cantidad de parametros que tiene
        send(socket, &instruction_type, sizeof(t_instruction_type), 0);
        send(socket, &parameters_count, sizeof(int), 0);

        // Ahora por cada parametro
        for(int j = 0; j < parameters_count; j++) 
        {
            t_parameter* param = list_get(inst->parameters, j);
            // Envio primero si es un string o no
            send(socket, &param->is_string, sizeof(bool), 0);

            // Si es un string envio un string
            if(param->is_string) {
                send_string(socket, (char*)param->parameter);
            }
            // Si no es un string lo envio como un numero
            else
            {
                send(socket, &param->parameter, sizeof(int), 0);
            }
        }
    }
}
// Recibe las instrucciones
t_list* recv_instructions(int socket)
{
    // Primero recibo el codigo de operacion
    op_code op_code;
    recv(socket, &op_code, sizeof(op_code), 0);

    // El codigo de operacion debe ser el correcto o no se puede
    // asegurar que los bytes que lleguen sean los correctos
    if(op_code != INSTRUCTIONS) {
        log_error(logger, "Se esperaba recibir la op_code %i, pero se recibio %i", INSTRUCTIONS, op_code);
        exit(EXIT_FAILURE);
    }

    t_list* instructions = list_create();

    // Los recv estan en el mismo orden que los send, por lo tanto
    // me aseguro que siempre reciba lo que necesito
    // Ahora recibo cuantas instrucciones se mandaron
    int instructions_count;
    recv(socket, &instructions_count, sizeof(int), 0);

    for(int i = 0; i < instructions_count; i++)
    {
        t_instruction* instruction = malloc(sizeof(t_instruction));
        int parameters_count;

        // Primero recibo el tipo de instruccion y me guardo en una
        // variable cuantos parametros trae, (en teoria nunca vendrian mas que 2
        // pero creo que esta piola hacerlo para cualquier cantidad)
        recv(socket, &instruction->instruction, sizeof(t_instruction_type), 0);
        recv(socket, &parameters_count, sizeof(int), 0);

        instruction->parameters = list_create();

        for(int j = 0; j < parameters_count; j++) 
        {
            bool is_string;
            recv(socket, &is_string, sizeof(bool), 0);

            t_parameter* parameter = malloc(sizeof(t_parameter));
            parameter->is_string = is_string;

            if(is_string)
            {
                char* string = recv_string(socket);
                parameter->parameter = (void*)string;
            }
            else
            {
                int number;
                recv(socket, &number, sizeof(int), 0);
                parameter->parameter = (void*)number;
            }
            list_add(instruction->parameters, (void*)parameter);
        }
        list_add(instructions, (void*)instruction);
    }
    
    return instructions;
}
