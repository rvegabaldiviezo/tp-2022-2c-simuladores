#include <commons/collections/list.h>

typedef enum {
    STRING,
    INSTRUCTIONS
} op_code;

void send_string(int socket, char* string);
char* recv_string(int socket);

void send_instructions(int socket, t_list* instructions);
t_list* recv_instructions(int socket);