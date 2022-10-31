#include <shared/structures.h>

void initialize_queues();

void new_state(t_pcb* pcb);
void ready_state(t_pcb* pcb);
void block_state(t_pcb* pcb);
void execute_algorithm();
void wait_cpu_dispatch();