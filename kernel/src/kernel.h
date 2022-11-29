#include <shared/structures.h>

void initialize_logger(char **argv);
void initialize_config(char **argv);
void initialize_sockets();
void create_process(int socket_consola, t_list* instructions, t_list* segments);