#include <commons/collections/list.h>
#include "structures.h"

#ifndef __SERIALIZATION_H
#define __SERIALIZATION_H

typedef enum {
    STRING,
    INSTRUCTIONS,
    PCB,
} op_code;

typedef struct {
    int size;
    void* stream;
} t_buffer;

void send_string(int socket, char* string);
char* recv_string(int socket);

void send_instructions(int socket, t_list* instructions);
t_list* recv_instructions(int socket);

void send_pcb(int socket, t_pcb* pcb);
t_pcb* recv_pcb(int socket);

// Para poder recibir y enviar que hubo una interrupcion por quantum entre el kernel y la cpu
// Desarrollado por ramon
void send_interrupt(int socket); // falta implementar
void recv_interrupt(int socket);
#endif