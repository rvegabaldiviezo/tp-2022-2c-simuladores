#include <stdbool.h>
typedef struct
{
	int memory_size;
	int page_size;
	int inputs_table;
	int memory_delay;
	int frames_per_process;
	int swap_delay;
	int swap_size;
    char* replace_algorithm;
    char* path_swap;
} t_memoria_config;

typedef struct
{
	int frame;
	bool P;
	bool U;
	bool M;
	int swap_pos;
} t_page_table_data;


void initialize_logger(argv);
void initialize_config(argv);
void initialize_sockets();
void initialize_memory_structures();
void handle_kernel();
void* handle_cpu(void* arg);
