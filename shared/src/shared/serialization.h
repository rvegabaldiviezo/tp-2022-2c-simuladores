#include <commons/collections/list.h>
#include "structures.h"

#ifndef __SERIALIZATION_H
#define __SERIALIZATION_H

typedef enum {
    STRING,
    INSTRUCTIONS,
    PCB,
    INTERRUPTION_QUANTUM,               // Se usa para enviarlo al cpu_interrupt y se recibe por el kernel en cpu_dispatch
    INTERRUPTION_IO,
    INTERRUPTION_EXECUTION_FINISHED,
    PAGE_FAULT
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

#endif