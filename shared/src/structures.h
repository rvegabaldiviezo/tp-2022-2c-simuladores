typedef enum {
	SET,
	ADD,
	MOV_IN,
	MOV_OUT,
	IO,
	EXIT
} t_operator;

typedef struct {
	t_operator operator;
	int param1;
	int param2;
} t_instruction;
