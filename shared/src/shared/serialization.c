#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/socket.h>
#include <commons/string.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include "serialization.h"

extern t_log* logger;

op_code recv_and_validate_op_code_is(int socket, op_code op_code_expected)
{
    // Primero recibo el codigo de operacion
    op_code op_code_received;
    recv(socket, &op_code_received, sizeof(op_code), 0);

    // El codigo de operacion debe ser el correcto o no se puede
    // asegurar que los bytes que lleguen sean los correctos
    if(op_code_received != op_code_expected) {
        log_error(logger, "Se esperaba recibir la op_code (%i), pero se recibio op_code (%i)", op_code_expected, op_code_received);
        exit(EXIT_FAILURE);
    }
    return op_code_received;
}
op_code recv_op_code(int socket)
{
    // Recibo el codigo de operacion
    op_code op_code_received;
    recv(socket, &op_code_received, sizeof(op_code), 0);
    return op_code_received;
}
int recv_int(int socket)
{
    int value;
    recv(socket, &value, sizeof(int), 0);
    return value;
}

/*
 * --- Funciones para facilitar el uso de buffers ---
 */ 
t_buffer* create_buffer()
{
    t_buffer* buffer = malloc(sizeof(t_buffer));
    buffer->size = 0;
    buffer->stream = NULL;
    return buffer;
}
void destroy_buffer(t_buffer* buffer)
{
    free(buffer->stream);
    free(buffer);
}
void add_to_buffer(t_buffer* buffer, void* magic, unsigned int size)
{
    buffer->stream = realloc(buffer->stream, buffer->size + size);
    memcpy(buffer->stream + buffer->size, magic, size);
    buffer->size += size;
}
void add_op_code(t_buffer* buffer, op_code operation_code)
{
    add_to_buffer(buffer, &operation_code, sizeof(op_code));
}
void add_string(t_buffer* buffer, char* string)
{
    // Agrego el codigo de operacion
    add_op_code(buffer, STRING);

    int length = string_length(string) + 1;

    add_to_buffer(buffer, &length, sizeof(int));
    add_to_buffer(buffer, string, sizeof(char) * length);
}
void add_instructions(t_buffer* buffer, t_list* instructions)
{
    // Agrego el codigo de operacion
    add_op_code(buffer, INSTRUCTIONS);

    int instructions_count = list_size(instructions);
    // Agrego un numero que representa cuantas instrucciones se mandan
    add_to_buffer(buffer, &instructions_count, sizeof(int));

    for(int i = 0; i < instructions_count; i++)
    {
        t_instruction* inst = list_get(instructions, i);

        t_instruction_type instruction_type = inst->instruction;
        int parameters_count = list_size(inst->parameters);

        // Agrego el tipo de instruccion (SET, ADD, etc)
        // Y la cantidad de parametros que tiene
        add_to_buffer(buffer, &instruction_type, sizeof(t_instruction_type));
        add_to_buffer(buffer, &parameters_count, sizeof(int));

        // Ahora por cada parametro
        for(int j = 0; j < parameters_count; j++) 
        {
            t_parameter* param = list_get(inst->parameters, j);
            // Agrego primero si es un string o no
            add_to_buffer(buffer, &param->is_string, sizeof(bool));

            // Si es un string agrego un string
            if(param->is_string) {
                add_string(buffer, (char*)param->parameter);
            }
            // Si no es un string lo agrego como un numero
            else
            {
                add_to_buffer(buffer, &param->parameter, sizeof(int));
            }
        }
    }
}

void add_registers(t_buffer* buffer, t_register* registers)
{
    add_to_buffer(buffer, &registers[AX], sizeof(uint32_t));
    add_to_buffer(buffer, &registers[BX], sizeof(uint32_t));
    add_to_buffer(buffer, &registers[CX], sizeof(uint32_t));
    add_to_buffer(buffer, &registers[DX], sizeof(uint32_t));
}

void send_buffer(int socket, t_buffer* buffer)
{
    send(socket, buffer->stream, buffer->size, 0);
}
/*
 * ------------------------------------------------
 */

// Envia a travez del socket un string
void send_string(int socket, char* string)
{
    t_buffer* buffer = create_buffer();
    add_string(buffer, string);
    send_buffer(socket, buffer);
    destroy_buffer(buffer);
}
// Recibe un string por el socket
// Se hace un malloc, por lo tanto no olvidarse de llamar FREE
// cuando no se use
char* recv_string(int socket)
{
    recv_and_validate_op_code_is(socket, STRING);

    int length;
    recv(socket, &length, sizeof(int), 0);

    char* string = malloc(sizeof(char) * length);
    recv(socket, string, sizeof(char) * length, 0);

    return string;
}
// Se envia una lista de instrucciones a traves de un socket
void send_instructions(int socket, t_list* instructions)
{
    t_buffer* buffer = create_buffer();
    add_instructions(buffer, instructions);
    send_buffer(socket, buffer);
    destroy_buffer(buffer);
}
// Recibe las instrucciones
t_list* recv_instructions(int socket)
{
    recv_and_validate_op_code_is(socket, INSTRUCTIONS);

    t_list* instructions = list_create();

    // Los recv estan en el mismo orden que los add_to_buffer, por lo tanto
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
// CPU -> Kernel
/**
 * Envia pcb, dispositivo en forma de string y un argumento (puede ser un registro o unidades de trabajo)
 */
void send_pcb_io(int socket, t_pcb* pcb, char* device, int arg)
{
    t_buffer* buffer = create_buffer();

    add_op_code(buffer, PCB);
    add_to_buffer(buffer, &pcb->id, sizeof(pcb->id));
    add_to_buffer(buffer, &pcb->interrupt_type, sizeof(pcb->interrupt_type));
    add_to_buffer(buffer, &pcb->program_counter, sizeof(pcb->program_counter));
    add_registers(buffer, pcb->registers);
    add_to_buffer(buffer, &pcb->socket_consola, sizeof(pcb->socket_consola));
    add_instructions(buffer, pcb->instructions);
    add_string(buffer, device);
    add_to_buffer(buffer, &arg, sizeof(arg));

    send_buffer(socket, buffer);
    destroy_buffer(buffer);
}
// Kernel -> CPU o CPU -> Kernel
void send_pcb(int socket, t_pcb* pcb)
{
    t_buffer* buffer = create_buffer();

    add_op_code(buffer, PCB);
    add_to_buffer(buffer, &pcb->id, sizeof(pcb->id));
    add_to_buffer(buffer, &pcb->interrupt_type, sizeof(pcb->interrupt_type));
    add_to_buffer(buffer, &pcb->program_counter, sizeof(pcb->program_counter));
    add_registers(buffer, pcb->registers);
    add_to_buffer(buffer, &pcb->socket_consola, sizeof(pcb->socket_consola));
    add_instructions(buffer, pcb->instructions);

    send_buffer(socket, buffer);
    destroy_buffer(buffer);
}
t_pcb* recv_pcb(int socket)
{
    recv_and_validate_op_code_is(socket, PCB);

    t_pcb* pcb = malloc(sizeof(t_pcb));
    recv(socket, &pcb->id, sizeof(pcb->id), 0);
    recv(socket, &pcb->interrupt_type, sizeof(pcb->interrupt_type), 0);
    recv(socket, &pcb->program_counter, sizeof(pcb->program_counter), 0);
    recv(socket, &pcb->registers[AX], sizeof(uint32_t), 0);
    recv(socket, &pcb->registers[BX], sizeof(uint32_t), 0);
    recv(socket, &pcb->registers[CX], sizeof(uint32_t), 0);
    recv(socket, &pcb->registers[DX], sizeof(uint32_t), 0);
    recv(socket, &pcb->socket_consola, sizeof(pcb->socket_consola), 0);
    pcb->instructions = recv_instructions(socket);

    return pcb;
}

void send_interrupt(int socket)
{
    int dummy = 1;
    send(socket, &dummy, sizeof(dummy), 0);
}
void recv_interrupt(int socket)
{
    int dummy;
    recv(socket, &dummy, sizeof(dummy), 0);
}

// Kernel -> Consola
void send_teclado(int socket)
{
    t_buffer* buffer = create_buffer();
    add_op_code(buffer, TECLADO);
    send_buffer(socket, buffer);
    destroy_buffer(buffer);
}
void send_pantalla(int socket, int value)
{
    t_buffer* buffer = create_buffer();
    add_op_code(buffer, PANTALLA);
    add_to_buffer(buffer, &value, sizeof(value));
    send_buffer(socket, buffer);
    destroy_buffer(buffer);
}
void send_exit(int socket)
{
    t_buffer* buffer = create_buffer();
    add_op_code(buffer, PROCESS_FINISHED);
    send_buffer(socket, buffer);
    destroy_buffer(buffer);
}
// Consola -> Kernel
void send_teclado_response(int socket, int value)
{
    t_buffer* buffer = create_buffer();
    add_op_code(buffer, TECLADO);
    add_to_buffer(buffer, &value, sizeof(value));
    send_buffer(socket, buffer);
    destroy_buffer(buffer);
}
// Consola -> Kernel
void send_pantalla_response(int socket)
{
    t_buffer* buffer = create_buffer();
    add_op_code(buffer, PANTALLA);
    send_buffer(socket, buffer);
    destroy_buffer(buffer);
}
void send_segments(int socket, t_list* segments)
{
    t_buffer* buffer = create_buffer();
    add_op_code(buffer, SEGMENTS);

    int segments_count = list_size(segments);
    add_to_buffer(buffer, &segments_count, sizeof(segments_count));
    for(int i = 0; i < segments_count; i++)
    {
        int segment = (int)list_get(segments, i);
        add_to_buffer(buffer, &segment, sizeof(segment));
    }
    send_buffer(socket, buffer);
    destroy_buffer(buffer);
}
t_list* recv_segments(int socket)
{
    recv_and_validate_op_code_is(socket, SEGMENTS);

    int segments_count = recv_int(socket);
    t_list* segments = list_create();
    for(int i = 0; i < segments_count; i++)
    {
        int segment = recv_int(socket);
        list_add(segments, segment);
    }
    return segments;
}

void send_memdata(int socket, int memory_size, int page_size)
{
    t_buffer* buffer = create_buffer();
    add_to_buffer(buffer, &memory_size, sizeof(memory_size));
    add_to_buffer(buffer, &page_size, sizeof(page_size));
    send_buffer(socket, buffer);
    destroy_buffer(buffer);
}

int recv_memory_size(int socket)
{
	int memory_size;
	recv(socket, &memory_size, sizeof(memory_size), 0);
	return memory_size;
}

int recv_page_size(int socket)
{
	int page_size;
	recv(socket, &page_size, sizeof(page_size), 0);
	return page_size;
}

void send_process_started(int socket, t_list* segments)
{
    t_buffer* buffer = create_buffer();
    add_op_code(buffer, PROCESS_STARTED);
    int segments_count = list_size(segments);
    add_to_buffer(buffer, &segments_count, sizeof(segments_count));
    for(int i = 0; i < segments_count; i++)
    {
        int segment = (int)list_get(segments, i);
        add_to_buffer(buffer, &segment, sizeof(segment));
    }
    send_buffer(socket, buffer);
    destroy_buffer(buffer);
}
t_list* recv_process_started(int socket)
{
    int segments_count = recv_int(socket);
    t_list* segments = list_create();
    for(int i = 0; i < segments_count; i++)
    {
        int segment = recv_int(socket);
        list_add(segments, segment);
    }
    return segments;
}
void send_process_finished(int socket, t_list* segments)
{
    t_buffer* buffer = create_buffer();
    add_op_code(buffer, PROCESS_FINISHED);
    int segments_count = list_size(segments);
    add_to_buffer(buffer, &segments_count, sizeof(segments_count));
    for(int i = 0; i < segments_count; i++)
    {
        t_segment* segment = (t_segment*)list_get(segments, i);
        add_to_buffer(buffer, &segment->size, sizeof(segment->size));
        add_to_buffer(buffer, &segment->page_table_index, sizeof(segment->page_table_index));
    }
    send_buffer(socket, buffer);
    destroy_buffer(buffer);
}
t_list* recv_process_finished(int socket)
{
    int segments_count = recv_int(socket);
    t_list* segments = list_create();
    for(int i = 0; i < segments_count; i++)
    {
        t_segment* segment = (t_segment*)malloc(sizeof(t_segment));
        segment->size = recv_int(socket);
        segment->page_table_index = recv_int(socket);
        list_add(segments, segment);
    }
    return segments;
}
// resolve -> aun no resuelto (resolvelo)
void send_page_fault_resolve(int socket, int segment, int page)
{
    t_buffer* buffer = create_buffer();
    add_op_code(buffer, PAGE_FAULT);
    add_to_buffer(buffer, &segment, sizeof(segment));
    add_to_buffer(buffer, &page, sizeof(page));
    send_buffer(socket, buffer);
    destroy_buffer(buffer);
}

// Memoria -> Kernel
void send_segment_table(int socket, t_list* segments)
{
    t_buffer* buffer = create_buffer();
    add_op_code(buffer, PROCESS_STARTED);
    int segments_count = list_size(segments);
    add_to_buffer(buffer, &segments_count, sizeof(segments_count));
    for(int i = 0; i < segments_count; i++)
    {
        t_segment* segment = (t_segment*)list_get(segments, i);
        add_to_buffer(buffer, &segment->size, sizeof(segment->size));
        add_to_buffer(buffer, &segment->page_table_index, sizeof(segment->page_table_index));
    }
    send_buffer(socket, buffer);
    destroy_buffer(buffer);
}
t_list* recv_segment_table(int socket)
{
    recv_and_validate_op_code_is(socket, PROCESS_STARTED);
    int segments_count = recv_int(socket);
    t_list* segments = list_create();
    for(int i = 0; i < segments_count; i++)
    {
        t_segment* segment = (t_segment*)malloc(sizeof(t_segment));
        segment->size = recv_int(socket);
        segment->page_table_index = recv_int(socket);
        list_add(segments, segment);
    }
    return segments;
}
// resolved -> resuelto 
void send_page_fault_resolved(int socket)
{
    t_buffer* buffer = create_buffer();
    add_op_code(buffer, PAGE_FAULT_RESOLVED);
    send_buffer(socket, buffer);
    destroy_buffer(buffer);
}

