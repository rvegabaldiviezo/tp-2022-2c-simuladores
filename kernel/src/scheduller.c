#include <pthread.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <signal.h>
#include <semaphore.h>
#include <commons/collections/queue.h>
#include <commons/collections/dictionary.h>
#include <commons/log.h>
#include <commons/string.h>
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
extern int socket_memoria_page_fault;
// Queues
t_queue* new_queue;
t_queue* ready_1_queue;
t_queue* ready_2_queue;
// IO Devices
t_dictionary* io_queues;
t_dictionary* io_times;
t_dictionary* io_semaphores;
typedef struct {
    t_pcb* pcb;
    char* device;
    int arg;
} t_io;
typedef struct {
    t_pcb* pcb;
    int segment;
    int page;
} t_pf;
// Semaphores
sem_t can_execute;
// Threads
pthread_t thread_interrupt;

void initialize_scheduller()
{
    new_queue = queue_create();
    ready_1_queue = queue_create();
    ready_2_queue = queue_create();
    io_queues = dictionary_create();
    io_semaphores = dictionary_create();
    io_times = dictionary_create();

    // Creo hilo para manejar page faults de la memoria
    pthread_t tid;
    pthread_create(&tid, NULL, handle_page_fault, NULL);

    char** config_devices = config_get_array_value(config, "DISPOSITIVOS_IO");
    char** config_devices_time = config_get_array_value(config, "TIEMPOS_IO");
    int i_device = 0;
    while(config_devices[i_device] != NULL) {
        // Creamos una pila para este dispositivo
        char* device = config_devices[i_device];
        int time = atoi(config_devices_time[i_device]);
        dictionary_put(io_queues, device, queue_create());
        dictionary_put(io_times, device, time);
        // Creamos un semaforo para este dispositivo (para indicar que se puede popear)
        sem_t* io_sem = malloc(sizeof(io_sem));
        sem_init(io_sem, 0, 0);
        dictionary_put(io_semaphores, device, io_sem);
        pthread_t tid; // no voy a usar el thread id asi que lo creo y muere
        pthread_create(&tid, NULL, handle_io, device);
        i_device++;
    }

    sem_init(&can_execute, 0, 0);
	log_trace(logger, "Scheduller inicializado");
}

void log_ready()
{
    char* pids;

    switch (scheduling_algorithm)
    {
    case FIFO:
    case RR:
        pids = string_new();
        for(int i = 0; i < queue_size(ready_1_queue); i++)
        {
            t_pcb* pcb = (t_pcb*)queue_pop(ready_1_queue);
            string_append_with_format(&pids, "%s%i", i == 0 ? "" : ", ", pcb->id);
            queue_push(ready_1_queue, pcb);
        }
        log_info(logger, "Cola Ready %s: [%s]", scheduling_algorithm == FIFO ? "FIFO" : "RR", pids);
        free(pids);
        break;
    
    default:
        pids = string_new();
        for(int i = 0; i < queue_size(ready_1_queue); i++)
        {
            t_pcb* pcb = (t_pcb*)queue_pop(ready_1_queue);
            string_append_with_format(&pids, "%s%i", i == 0 ? "" : ", ", pcb->id);
            queue_push(ready_1_queue, pcb);
        }
        log_info(logger, "Cola Ready 1 FEEDBACK: [%s]", pids);
        free(pids);

        pids = string_new();
        for(int i = 0; i < queue_size(ready_2_queue); i++)
        {
            t_pcb* pcb = (t_pcb*)queue_pop(ready_2_queue);
            string_append_with_format(&pids, "%s%i", i == 0 ? "" : ", ", pcb->id);
            queue_push(ready_2_queue, pcb);
        }
        log_info(logger, "Cola Ready 2 FEEDBACK: [%s]", pids);
        free(pids);
        break;
    }
}

void* start_schedulling(void* arg)
{
    log_trace(logger, "Se crea hilo para SCHEDULLER");
	execute_algorithm();
	wait_cpu_dispatch();
}

void new_state(t_pcb* pcb)
{
    log_info(logger, "Se crea el proceso %i en NEW", pcb->id);
    if(queue_size(ready_1_queue) < max_degree_multiprogramming) 
    {
        queue_push(ready_1_queue, pcb);
        log_info(logger, "PID: %i - Estado Anterior: NEW - Estado Actual: READY", pcb->id);
    }
    else
    {
        queue_push(new_queue, pcb);
    }
    sem_post(&can_execute);
}

void ready_state_from_quantum(t_pcb* pcb)
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
    log_info(logger, "PID: %i - Desalojado por fin de Quantum", pcb->id);
    log_ready();
    sem_post(&can_execute);
}

void ready_state_from_io(t_pcb* pcb)
{
    queue_push(ready_1_queue, pcb);
    log_info(logger, "PID: %i - Estado Anterior: BLOCKED - Estado Actual: READY", pcb->id);
    log_ready();
    sem_post(&can_execute);
}

void ready_state_from_page_fault(t_pcb* pcb)
{
    queue_push(ready_1_queue, pcb);
    log_info(logger, "PID: %i - Estado Anterior: BLOCKED - Estado Actual: READY", pcb->id);
    log_ready();
    sem_post(&can_execute);
}

void block_state(t_pcb* pcb, char* device, int arg)
{
    t_io* io_data = malloc(sizeof(t_io));
    io_data->pcb = pcb;
    io_data->device = device;
    io_data->arg = arg;

    if(strcmp(device, "TECLADO") == 0)
    {
        // Creo un hilo para manejar teclado
        log_trace(logger, "Se crea hilo para manejar TECLADO");
        pthread_t tid;
        pthread_create(&tid, NULL, handle_teclado, io_data);
    }
    else if(strcmp(device, "PANTALLA") == 0)
    {
        // Creo un hilo para manejar pantalla
        log_trace(logger, "Se crea hilo para manejar PANTALLA");
        pthread_t tid;
        pthread_create(&tid, NULL, handle_pantalla, io_data);
    }
    else
    {
        // Para el resto de los dispositivos, se encolan
        t_queue* device_queue = (t_queue*)dictionary_get(io_queues, device);
        sem_t* io_sem = (sem_t*)dictionary_get(io_semaphores, device);

        queue_push(device_queue, io_data);
        log_trace(logger, "Encolo en dispositivo: %s", device);
        sem_post(io_sem);
    }
}

// Hilo para manejar un page fault
void* handle_page_fault(void* arg)
{    
    while(true)
    {
        // Esperamos que memoria envie un page fault resuelto
        recv_and_validate_op_code_is(socket_memoria_page_fault, PAGE_FAULT_RESOLVED);
        t_pcb* pcb = recv_pcb(socket_memoria_page_fault);
        // Enviamos el proceso a READY
        ready_state_from_page_fault(pcb);
        log_debug(logger, "Se resolvio el PAGE FAULT de PID: %i", pcb->id);
    }    
}

// TODO PANTALLA y TECLADO tienen que ejecutar sin encolarse
void* handle_pantalla(void* arg)
{
    t_io* io_data = (t_io*)arg;
    t_pcb* pcb = io_data->pcb;
    t_register reg = (t_register)io_data->arg;

    log_debug(logger, "PID: %i - Envio a consola N°%i que muestre %i por PANTALLA", pcb->id, pcb->socket_consola, pcb->registers[reg]);
    // Aviso a consola que muestre algo por pantalla
    send_pantalla(pcb->socket_consola, pcb->registers[reg]);
    // Espero a que la consola me confirme que llego una pantalla
    recv_and_validate_op_code_is(pcb->socket_consola, PANTALLA);
    log_debug(logger, "PID: %i - Recibi de consola N°%i confirmacion que se mostro el %i por PANTALLA", pcb->id, pcb->socket_consola, pcb->registers[reg]);
    // Desbloqueamos el proceso
    ready_state_from_io(io_data->pcb);
}

void* handle_teclado(void* arg)
{
    t_io* io_data = (t_io*)arg;
    t_pcb* pcb = io_data->pcb;
    t_register reg = (t_register)io_data->arg;

    // Aviso a consola que escriba algo por teclado
    log_debug(logger, "PID: %i - Envio a consola N°%i a que escriba un valor por TECLADO", pcb->id, pcb->socket_consola);
    send_teclado(pcb->socket_consola);
    // Me aseguro que la consola haya devuelto una respuesta por TECLADO
    recv_and_validate_op_code_is(pcb->socket_consola, TECLADO);
    // Recivo el valor del teclado
    int value = recv_int(pcb->socket_consola);
    log_debug(logger, "PID: %i - Recibi de consola N°%i el valor %i por TECLADO", pcb->id, pcb->socket_consola, value);
    // Lo guardo en el registro indicado por CPU
    pcb->registers[reg] = value;
    // Desbloqueamos el proceso
    ready_state_from_io(io_data->pcb);
}

void* handle_io(void* arg)
{
    char* device = (char*)arg;
    log_trace(logger, "Se crea hilo para cola de %s", device);
    t_queue* device_queue = (t_queue*)dictionary_get(io_queues, device);
    sem_t* io_sem = (sem_t*)dictionary_get(io_semaphores, device);
    int time = (int)dictionary_get(io_times, device);
    
    while(true) {
        sem_wait(io_sem);
        t_io* io_data = (t_io*)queue_pop(device_queue);
        log_debug(logger, "Dispositivo %s usado por PID: %i", device, io_data->pcb->id);
        usleep(io_data->arg * time * 1000);
        // Se termino de resolver el IO
        // Desbloqueamos el proceso
        ready_state_from_io(io_data->pcb);
    }
}

/**
 * Ejecuta algoritmo de planificacion definido por archivo de configuracion
 */
void execute_algorithm()
{
    log_trace(logger, "Esperando a que llegue un proceso en READY o NEW...");
    sem_wait(&can_execute);

    bool use_quantum = scheduling_algorithm == RR;

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
            log_info(logger, "PID: %i - Estado Anterior: NEW - Estado Actual: READY", pcb->id);
            log_info(logger, "PID: %i - Estado Anterior: READY - Estado Actual: EXECUTE", pcb->id);
        }

        break;
    case FEEDBACK:
        if(queue_size(ready_1_queue) > 0) {
            pcb = (t_pcb*)queue_pop(ready_1_queue);
            log_info(logger, "PID: %i - Estado Anterior: READY - Estado Actual: EXECUTE", pcb->id);
            use_quantum = true;
        }
        else if(queue_size(ready_2_queue) > 0) {
            pcb = (t_pcb*)queue_pop(ready_2_queue);
            log_info(logger, "PID: %i - Estado Anterior: READY - Estado Actual: EXECUTE", pcb->id);
        }
        else if(queue_size(new_queue) > 0) {
            pcb = (t_pcb*)queue_pop(new_queue);
            log_info(logger, "PID: %i - Estado Anterior: NEW - Estado Actual: READY", pcb->id);
            log_info(logger, "PID: %i - Estado Anterior: READY - Estado Actual: EXECUTE", pcb->id);
        }
        break;
    }

    send_pcb(socket_cpu_dispatch, pcb);
    // Empezamos el thread de quantum
    if(use_quantum) {
	    pthread_create(&thread_interrupt, NULL, start_quantum, NULL); // thread interrupt
    }
}

void wait_cpu_dispatch()
{
    while(true) 
    {
        // Se espera a recibir el pcb de la cpu porque termino de ejecutar
        t_pcb* pcb = recv_pcb(socket_cpu_dispatch);
        // Ya no hace falta el fin de quantum
        if(scheduling_algorithm != FIFO) {
            if(pthread_cancel(thread_interrupt) == 0) {
                log_debug(logger, "Thread INTERRUPT destruido");
            } else {
                log_error(logger, "No se pudo cancelar el thread de INTERRUPT");
            }
        }
        // verificamos que hacer (porque nos envio el pcb la cpu)
        switch(pcb->interrupt_type) {
            case EXECUTION_FINISHED:
                // avisamos a la memoria a que libere los datos del proceso
                log_trace(logger, "segment_table: %i", list_size(pcb->segment_table));
                send_process_finished(socket_memoria, pcb);
                recv_and_validate_op_code_is(socket_memoria, PROCESS_FINISHED);
                // avisamos a la consola de que se finalizo el proceso
                send_exit(pcb->socket_consola);
                log_info(logger, "PID: %i - Estado Anterior: EXECUTE - Estado Actual: EXIT", pcb->id);
                break;
            case INT_QUANTUM:
                // metemos el pcb en la cola de ready
                ready_state_from_quantum(pcb);
                break;
            case INT_IO:
                // obtenemos el dispositivo y registro o unidad de trabajo que tambien envio la cpu
                char* device = recv_string(socket_cpu_dispatch);
                int arg = recv_int(socket_cpu_dispatch);
                log_info(logger, "PID: %i - Estado Anterior: EXECUTE - Estado Actual: BLOCKED", pcb->id);
                log_info(logger, "PID: %i - Bloqueado por: %s", pcb->id, device);
                // resolver la solicitud de i/o uno de los hilos
                block_state(pcb, device, arg);
                break;
            case INT_PAGE_FAULT:
                // Resolvemos el pagefault de la siguiente manera
                /** 
                    1.- Mover al proceso al estado Bloqueado. Este estado bloqueado será independiente de todos los demás ya que solo afecta al proceso y no compromete recursos compartidos.
                    2.- Solicitar al módulo memoria que se cargue en memoria principal la página correspondiente, la misma será obtenida desde el mensaje recibido de la CPU.
                    3.- Esperar la respuesta del módulo memoria.
                    4.- Al recibir la respuesta del módulo memoria, desbloquear el proceso y colocarlo en la cola de ready correspondiente.
                */
                int segment = recv_int(socket_cpu_dispatch);
                int page = recv_int(socket_cpu_dispatch);

                log_info(logger, "PID: %i - Estado Anterior: EXECUTE - Estado Actual: BLOCKED", pcb->id);
                log_info(logger, "Page Fault PID: %i - Segmento: %i - Pagina: %i", pcb->id, segment, page);

                // Envio request a la memoria para que resuelva el page fault
                send_page_fault_resolve(socket_memoria, pcb, segment, page);

                break;
            case SEGMENTATION_FAULT:
                // Ocurrio un segfault
                log_debug(logger, "Ocurrio un SEGMENTATION_FAULT");
                send_segmentation_fault(pcb->socket_consola);

            default:
                break;
        }
        execute_algorithm();
    }
}

// Desarrolla por ramon
/**
 * Se debe esperar por tiempo de quantum y enviar interrupcion por quantum a la cpu
 */
// Cuando no hay ningun proceso a ejecutar no se debe enviar interrupciones
// Cuando llega un proceso recien ahi empieza a contar
void* start_quantum(void* arg)
{
    log_trace(logger, "Se crea hilo para INTERRUPT");
    usleep(quantum_rr * 1000);
    send_interrupt(socket_cpu_interrupt);
    log_trace(logger, "Envie interrupcion por Quantum tras %i milisegundos", quantum_rr);
}
