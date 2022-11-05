#include <shared/structures.h>

void initialize_scheduller();

void new_state(t_pcb* pcb);
void ready_state(t_pcb* pcb);
void block_state(t_pcb* pcb);
void* start_schedulling(void* arg);
void execute_algorithm();
void wait_cpu_dispatch();
void* quantum_time(void* arg);