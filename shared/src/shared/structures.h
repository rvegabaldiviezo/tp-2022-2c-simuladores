#include <stdint.h>
#include <commons/collections/list.h>

#ifndef __STRUCTURES_H
#define __STRUCTURES_H

typedef enum {
	SET,
	ADD,
	MOV_IN,
	MOV_OUT,
	IO,
	EXIT
} t_instruction_type;

typedef enum {
	AX,
	BX,
	CX,
	DX,
} t_register;

typedef struct {
	bool is_string;
	void* parameter;
} t_parameter;

typedef struct {
	t_instruction_type instruction;
	t_list* parameters;
} t_instruction;

// Proposicion
typedef enum {
    INT_QUANTUM,               // Se usa para enviarlo al cpu_interrupt y se recibe por el kernel en cpu_dispatch
    INT_IO,
	INT_PAGE_FAULT,
    EXECUTION_FINISHED,
	NO_INTERRUPT
} t_interrupt_type;

typedef struct {
	int id;
	// Agregar una nueva variable que defina razon de interrupt
	t_interrupt_type interrupt_type;
	unsigned int process_size;
	unsigned int program_counter;
	uint32_t registers[4];
	unsigned int page_table;
	double estimated_burst;
	int socket_consola;
	double start_burst;
	double estimated_remaining_burst;
	double execution_time;
	t_list* instructions;
} t_pcb;

#endif
