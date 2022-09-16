#include <commons/collections/list.h>

typedef enum {
	SET,
	ADD,
	MOV_IN,
	MOV_OUT,
	IO,
	EXIT
} t_operation;

typedef enum {
	AX,
	BX,
	CX,
	DX,
} t_register;

typedef struct {
	t_operation operation;
	t_list* parameters;
} t_instruction;
