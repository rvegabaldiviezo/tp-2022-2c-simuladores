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
t_queue* block_queue;
// Config
int max_level_multiprogramming;

void initialize_queues()
{
    new_queue = queue_create();
    fifo_ready_queue = queue_create();
    rr_ready_queue = queue_create();
    block_queue = queue_create();
}

void new_state(t_pcb* pcb)
{
    log_info(logger, "Se crea el proceso %i en NEW", pcb->id);
    queue_push(new_queue, pcb);
}

void ready_state(t_pcb* pcb)
{

}

void block_state(t_pcb* pcb)
{

}

void execute_algorithm()
{
    // nos fijamos que algoritmo de planificacion usamos
    // revisamos las colas correspondientes y "popeamos" un pcb de alguna de las colas
    // enviamos el pcb a la cpu por cpu_dispatch
}

void wait_cpu_dispatch()
{
    while(true) 
    {
        // Se espera a recibir el pcb de la cpu porque termino de ejecutar
        t_pcb* pcb; // Recibios el pcb de la cpu
        // verificamos que hacer (porque nos envio el pcb la cpu)
        op_code op_code_enviado_por_cpu;
        switch(op_code_enviado_por_cpu) {
            case INTERRUPTION_EXECUTION_FINISHED:
                // metemos el pcb en la cola de exit
                execute_algorithm();
                break;
            case INTERRUPTION_QUANTUM:
                // metemos el pcb en la cola de ready
                execute_algorithm();
                break;
            case INTERRUPTION_IO:
                // metemos el pcb en bloqueado
                execute_algorithm();
                break;
            case PAGE_FAULT:
                // Resolvemos el pagefault de la siguiente manera
                /** 
                    1.- Mover al proceso al estado Bloqueado. Este estado bloqueado será independiente de todos los demás ya que solo afecta al proceso y no compromete recursos compartidos.
                    2.- Solicitar al módulo memoria que se cargue en memoria principal la página correspondiente, la misma será obtenida desde el mensaje recibido de la CPU.
                    3.- Esperar la respuesta del módulo memoria.
                    4.- Al recibir la respuesta del módulo memoria, desbloquear el proceso y colocarlo en la cola de ready correspondiente.
                */
                break;
        }
    }
}

// Desarrolla por ramon
/**
 * Se debe esperar por tiempo de quantum y enviar interrupcion por quantum a la cpu
 */
void quantum_time()
{
    
}