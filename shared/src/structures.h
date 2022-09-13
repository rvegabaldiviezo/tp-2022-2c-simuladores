#include <commons/collections/list.h>

typedef enum {
	SET,
	ADD,
	MOV_IN,
	MOV_OUT,
	IO,
	EXIT
} t_operator;

typedef enum {
	AX,
	BX,
	CX,
	DX,
	DISCO
} t_parameter;

typedef struct {
	t_operator operator;
	t_list* parameters;
} t_instruction;
