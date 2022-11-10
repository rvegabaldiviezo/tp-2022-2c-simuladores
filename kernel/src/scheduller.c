#include <pthread.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <semaphore.h>
#include <commons/collections/queue.h>
#include <commons/collections/dictionary.h>
#include <commons/log.h>
#include <commons/config.h>
#include <shared/structures.h>
#include <shared/serialization.h>
#include <shared/log_extras.h>
#include "scheduller.h"

// Logger
extern t_log* logger;
// Config
extern t_config* config;
// Config values
extern int max_degree_multiprogramming;
extern t_scheduling_algorithm scheduling_algorithm;
extern int quantum_rr;
// Connections
extern int socket_cpu_interrupt;
extern int socket_cpu_dispatch;
extern int socket_memoria;
// Queues
t_queue* new_queue;
t_queue* ready_1_queue;
t_queue* ready_2_queue;
t_queue* block_queue;
// IO Devices
t_dictionary* io_devices;
// Semaphores
sem_t can_execute;

void initialize_scheduller()
{
    new_queue = queue_create();
    ready_1_queue = queue_create();
    ready_2_queue = queue_create();
    block_queue = queue_create();
    io_devices = dictionary_create();

    char** config_devices = config_get_array_value(config, "DISPOSITIVOS_IO");
    int i_device = 0;
    while(config_devices[i_device] != NULL) {
        char* device = config_devices[i_device];
        dictionary_put(io_devices, device, queue_create());
        log_trace(logger, "Genero pila para dispositivo %s", device);
        i_device++;
    }

    sem_init(&can_execute, 0, 0);
}

void* start_schedulling(void* arg)
{
	execute_algorithm();
	wait_cpu_dispatch();
}

void new_state(t_pcb* pcb)
{

    if(queue_size(ready_1_queue) < max_degree_multiprogramming) 
    {
        queue_push(ready_1_queue, pcb);
        log_info(logger, "PID: %i - Estado Anterior: NEW - Estado Actual: READY", pcb->id);
    }
    else
    {
        queue_push(new_queue, pcb);
        log_info(logger, "Se crea el proceso %i en NEW", pcb->id);
    }
    sem_post(&can_execute);
}

void ready_state(t_pcb* pcb)
{
    switch (scheduling_algorithm)
    {
    case FIFO:
    case RR:
        queue_push(ready_1_queue, pcb);
        break;
    
    case FEEDBACK:
        queue_push(ready_2_queue, pcb);
        break;
    }

    log_info(logger, "PID: %i - Estado Anterior: EXECUTE - Estado Actual: READY", pcb->id);
    sem_post(&can_execute);
}

void block_state(t_pcb* pcb)
{

}

typedef struct {
    t_pcb* pcb;
    char* device;
    int arg;
} t_io_thread_argument;

void* handle_io(void* arg)
{

}

void create_io_thread(t_pcb* pcb, char* device, int arg)
{
    t_io_thread_argument* io_thread_argument = malloc(sizeof(t_io_thread_argument));
    io_thread_argument->pcb = pcb;
    io_thread_argument->device = device;
    io_thread_argument->arg = arg;

    // TODO crear hilos
    //pthread_create(NULL, )
}

/**
 * Ejecuta algoritmo de planificacion definido por archivo de configuracion
 */
void execute_algorithm()
{
    log_trace(logger, "Esperando a que llegue un proceso en READY o NEW...");
    sem_wait(&can_execute);

    t_pcb* pcb;
    switch (scheduling_algorithm)
    {
    case FIFO:
    case RR:
        if(queue_size(ready_1_queue) > 0) {
            pcb = (t_pcb*)queue_pop(ready_1_queue);
            log_info(logger, "PID: %i - Estado Anterior: READY - Estado Actual: EXECUTE", pcb->id);
        }
        else if(queue_size(new_queue) > 0) {
            pcb = (t_pcb*)queue_pop(new_queue);
            log_info(logger, "PID: %i - Estado Anterior: NEW - Estado Actual: EXECUTE", pcb->id);
        }

        break;
    case FEEDBACK:
        if(queue_size(ready_1_queue) > 0) {
            pcb = (t_pcb*)queue_pop(ready_1_queue);
            log_info(logger, "PID: %i - Estado Anterior: READY - Estado Actual: EXECUTE", pcb->id);
        }
        else if(queue_size(ready_2_queue) > 0) {
            pcb = (t_pcb*)queue_pop(ready_2_queue);
            log_info(logger, "PID: %i - Estado Anterior: READY - Estado Actual: EXECUTE", pcb->id);
        }
        else if(queue_size(new_queue) > 0) {
            pcb = (t_pcb*)queue_pop(new_queue);
            log_info(logger, "PID: %i - Estado Anterior: NEW - Estado Actual: EXECUTE", pcb->id);
        }
        break;
    }

    send_pcb(socket_cpu_dispatch, pcb);
    
}

void wait_cpu_dispatch()
{
    while(true) 
    {
        // Se espera a recibir el pcb de la cpu porque termino de ejecutar
        t_pcb* pcb = recv_pcb(socket_cpu_dispatch);
        // verificamos que hacer (porque nos envio el pcb la cpu)
        switch(pcb->interrupt_type) {
            case EXECUTION_FINISHED:
                // avisamos a la consola de que se finalizo el proceso?
                log_info(logger, "PID: %i - Estado Anterior: EXECUTE - Estado Actual: EXIT", pcb->id);
                execute_algorithm();
                break;
            case INT_QUANTUM:
                // metemos el pcb en la cola de ready
                ready_state(pcb);
                execute_algorithm();
                break;
            case INT_IO:
                // obtenemos el dispositivo y registro o unidad de trabajo que tambien envio la cpu
                char* device = recv_string(socket_cpu_dispatch);
                int arg;
                recv(socket_cpu_dispatch, &arg, sizeof(arg), 0);
                log_info(logger, "PID: %i - Bloqueado por: %s", pcb->id, device);
                log_trace(logger, "Con argumento: %i", arg);
                // metemos el pcb en bloqueado
                block_state(pcb);
                // resolver la solicitud de i/o
                // acordarse de implentar un sem_post(&can_execute); al desbloquearse
                execute_algorithm();
                break;
            case INT_PAGE_FAULT:
                // Resolvemos el pagefault de la siguiente manera
                /** 
                    1.- Mover al proceso al estado Bloqueado. Este estado bloqueado será independiente de todos los demás ya que solo afecta al proceso y no compromete recursos compartidos.
                    2.- Solicitar al módulo memoria que se cargue en memoria principal la página correspondiente, la misma será obtenida desde el mensaje recibido de la CPU.
                    3.- Esperar la respuesta del módulo memoria.
                    4.- Al recibir la respuesta del módulo memoria, desbloquear el proceso y colocarlo en la cola de ready correspondiente.
                */
                break;
            default:
                break;
        }
    }
}

// Desarrolla por ramon
/**
 * Se debe esperar por tiempo de quantum y enviar interrupcion por quantum a la cpu
 */
void* start_quantum(void* arg)
{
    if(scheduling_algorithm == FIFO) {
        return;
    }

    while(true)
    {
        sleep((int)(quantum_rr * 0.001));
        send_interrupt(socket_cpu_interrupt);
        log_trace(logger, "Envio interrupcion por Quantum, espero %f segundos", (int)(quantum_rr * 0.001));
    }
}