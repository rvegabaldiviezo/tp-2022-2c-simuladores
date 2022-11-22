#include <commons/collections/list.h>
#include "structures.h"

#ifndef __SERIALIZATION_H
#define __SERIALIZATION_H

typedef enum {
    STRING,
    INSTRUCTIONS,
    SEGMENTS,
    PCB,
    TECLADO,
    PANTALLA,
    PROCESS_STARTED,
    PROCESS_FINISHED,
    PAGE_FAULT,
    PAGE_FAULT_RESOLVED,
    PAGE_TABLE_ACCESS,
    RAM_ACCESS
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
void send_segments(int socket, t_list* segments);
t_list* recv_segments(int socket);

void send_pcb_io(int socket, t_pcb* pcb, char* device, int arg);
void send_pcb(int socket, t_pcb* pcb);
t_pcb* recv_pcb(int socket);

// Para poder recibir y enviar que hubo una interrupcion por quantum entre el kernel y la cpu
// Desarrollado por ramon
void send_interrupt(int socket);
void recv_interrupt(int socket);

// Kernel -> Consola
void send_teclado(int socket);
void send_pantalla(int socket, int value);
void send_exit(int socket);
// Consola -> Kernel
void send_teclado_response(int socket, int value);
void send_pantalla_response(int socket);
// Memoria -> CPU
void send_memdata(int socket, int memory_size, int page_size);
int recv_memory_size(int socket);
int recv_page_size(int socket);

// Kernel -> Memoria
void send_process_started(int socket, t_list* segments);
t_list* recv_process_started(int socket);
void send_process_finished(int socket, t_list* segments);
t_list* recv_process_finished(int socket);
// resolve -> aun no resuelto 
void send_page_fault_resolve(int socket, int segment, int page);

// Memoria -> Kernel
void send_segment_table(int socket, t_list* segments);
t_list* recv_segment_table(int socket);
// resolved -> resuelto 
void send_page_fault_resolved(int socket);


#endif
