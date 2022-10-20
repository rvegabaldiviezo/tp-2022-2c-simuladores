#include <commons/collections/list.h>

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

typedef struct {
	int id;
	unsigned int process_size;
	unsigned int program_counter;
	unsigned int page_table;
	double estimated_burst;
	int socket_consola;
	double start_burst;
	double estimated_remaining_burst;
	double execution_time;
	t_list* instructions;
} t_pcb;
