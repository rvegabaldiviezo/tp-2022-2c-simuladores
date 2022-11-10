#include <commons/collections/list.h>
#include "structures.h"

#ifndef __SERIALIZATION_H
#define __SERIALIZATION_H

typedef enum {
    STRING,
    INSTRUCTIONS,
    PCB,
    TECLADO,
    PANTALLA,
    EXIT_EXECUTION
} op_code;

typedef struct {
    int size;
    void* stream;
} t_buffer;

op_code recv_and_validate_op_code_is(int socket, op_code op_code_expected);
op_code recv_op_code(int socket);

int recv_int(int socket);

void send_string(int socket, char* string);
char* recv_string(int socket);

void send_instructions(int socket, t_list* instructions);
t_list* recv_instructions(int socket);

void send_pcb_io(int socket, t_pcb* pcb, char* device, int arg);
void send_pcb(int socket, t_pcb* pcb);
t_pcb* recv_pcb(int socket);

// Para poder recibir y enviar que hubo una interrupcion por quantum entre el kernel y la cpu
// Desarrollado por ramon
void send_interrupt(int socket); // falta implementar
void recv_interrupt(int socket);

// Kernel -> Consola
void send_teclado(int socket);
void send_pantalla(int socket, int value);
void send_exit(int socket);
// Consola -> Kernel
void send_teclado_response(int socket, int value);
void send_pantalla_response(int socket);
#endif