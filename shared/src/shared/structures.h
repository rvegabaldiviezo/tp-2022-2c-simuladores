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
