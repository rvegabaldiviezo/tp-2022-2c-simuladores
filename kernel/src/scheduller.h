#include <shared/structures.h>

void initialize_scheduller();

void new_state(t_pcb* pcb);
void ready_state_from_quantum(t_pcb* pcb);
void ready_state_from_io(t_pcb* pcb);
void block_state(t_pcb* pcb, char* device, int arg);
void* start_schedulling(void* arg);
void execute_algorithm();
void wait_cpu_dispatch();
void* start_quantum(void* arg);
void* handle_io(void* arg);