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

typedef enum {
	FIFO,
	RR,
	FEEDBACK
} t_scheduling_algorithm;

// Proposicion
typedef enum {
    INT_QUANTUM,               // Se usa para enviarlo al cpu_interrupt y se recibe por el kernel en cpu_dispatch
    INT_IO,
	INT_PAGE_FAULT,
    EXECUTION_FINISHED,
	NO_INTERRUPT,
	SEGMENTATION_FAULT
} t_interrupt_type;

typedef struct {
	int size;
	int page_table_index;
} t_segment;

typedef struct {
	int id;
	// Agregar una nueva variable que defina razon de interrupt
	t_interrupt_type interrupt_type;
	unsigned int program_counter;
	uint32_t registers[4];
	int socket_consola;
	t_list* segment_table;
	t_list* instructions;
} t_pcb;

#endif
