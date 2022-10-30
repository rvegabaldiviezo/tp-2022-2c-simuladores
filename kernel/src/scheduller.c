#include <commons/collections/queue.h>
#include <commons/log.h>
#include <commons/config.h>
#include <shared/structures.h>

// Logger
extern t_log* logger;
// Config
extern t_config* config;
// Connections
extern int socket_cpu_interrupt;
extern int socket_cpu_dispatch;
extern int socket_memoria;
// Queues
t_queue* new_queue;
t_queue* fifo_ready_queue;
t_queue* rr_ready_queue;
t_queue* execute_queue;
t_queue* block_queue;
t_queue* exit_queue;
// Config
int max_level_multiprogramming;

void initialize_queues()
{
    new_queue = queue_create();
    fifo_ready_queue = queue_create();
    rr_ready_queue = queue_create();
    execute_queue = queue_create();
    block_queue = queue_create();
    exit_queue = queue_create();
}

void new_state(t_pcb* pcb)
{
    log_info(logger, "Se crea el proceso %i en NEW", pcb->id);
}

void ready_state(t_pcb* pcb)
{

}

void execute_state(t_pcb* pcb)
{

}

void block_state(t_pcb* pcb)
{

}

void exit_state(t_pcb* pcb)
{

}