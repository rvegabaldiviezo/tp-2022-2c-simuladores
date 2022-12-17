#include <math.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/bitarray.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <shared/structures.h>
#include <shared/serialization.h>
#include "memoria.h"
#include "handle_kernel.h"

extern t_log* logger;
extern int socket_kernel;
extern int socket_cpu_tlb;
extern int socket_kernel_page_fault;
extern t_memoria_config* memoria_config;

// Structures
extern void* ram;
extern FILE* swap;
extern t_list* page_tables;
extern t_list* frames_usage;
extern t_list* last_page_table_reference;
extern int global_time;

// Mutex
extern pthread_mutex_t ram_mutex;

t_page_table_data* victim;

int page_table_pointer = 0;
int page_pointer = 0;

void handle_kernel()
{
    while(true) 
    {
        recv_buffer_size(socket_kernel);
        op_code op_code = recv_op_code(socket_kernel);

        switch (op_code)
        {
        case PROCESS_STARTED:
            process_started();
            break;
        case PROCESS_FINISHED:
            process_finished();
            break;
        case PAGE_FAULT:
            resolve_page_fault();
            break;
        default:
			log_error(logger, "op_code invalido para Handle Kernel, se recibio %i", op_code);
			exit(EXIT_FAILURE);
        }
    }
}

// Creacion de tabla de paginas
void process_started()
{
	int pid = recv_int(socket_kernel);
    t_list* segments_sizes = recv_process_started(socket_kernel);
    t_list* segment_table = list_create();

    // Iteramos por cada segmento
    for(int i = 0; i < list_size(segments_sizes); i++)
    {
        int segment_size = list_get(segments_sizes, i);

        t_list* page_table = list_create();

        // Calculamos cuantas paginas necesitamos para el segmento
        int page_ammount = ceil((float)segment_size / (float)memoria_config->page_size);
        log_trace(logger, "Inicializo cant paginas: %i, para PID: %i", page_ammount, pid);

        // Creamos cada una de las paginas
        for(int j = 0; j < page_ammount; j++)
        {
            t_page_table_data* page = (t_page_table_data*)malloc(sizeof(t_page_table_data));

            page->pid = pid;
            page->frame = -1;
            page->P = 0;
            page->U = 0;
            page->M = 0;
            page->swap_pos = -1;
            page->timestamp = 0;

            log_trace(logger, "Page: %i | Frame: %i | P:%i | U:%i | M:%i", j, page->frame, page->P, page->U, page->M);
 
            list_add(page_table, page);

            t_last_page_table_data* last_page_table_data = (t_last_page_table_data*)malloc(sizeof(t_last_page_table_data));
            last_page_table_data->last_page = 0;
            last_page_table_data->last_page_table = 0;

            list_add_in_index(last_page_table_reference, pid, last_page_table_data);
        }

        // Guardamos la tabla de pagina en nuestra lista de tablas de paginas
        int page_table_index = list_add(page_tables, page_table);

        // Ya tenemos la tabla de paginas inicializada
        t_segment* segment = (t_segment*)malloc(sizeof(t_segment));
        segment->page_table_index = page_table_index;
        segment->size = segment_size;

        // Guardamos este segmento en la tabla de segmentos
        list_add(segment_table, segment);

        log_info(logger, "Creacion de Tabla de Paginas PID: %i - Segmento: %i - TAMAÑO: %i paginas", pid, i, page_ammount);
    }

    send_segment_table(socket_kernel, segment_table);
    list_destroy(segments_sizes);
    list_destroy(segment_table);
}
//void send_process_finished(int socket, int pid, t_list* segments);
void process_finished()
{
	t_pcb* pcb = recv_pcb(socket_kernel);
    t_list* segment_table = pcb->segment_table;

    for(int i = 0; i < list_size(segment_table); i++)
    {
        t_segment* segment = (t_segment*)list_get(segment_table, i);
        t_list* page_table = list_get(page_tables, segment->page_table_index);

        for(int j = 0; j < list_size(page_table); j++)
        {
            t_page_table_data* page = (t_page_table_data*)list_get(page_table, j);

            if(page->P == 1) {
                list_replace(frames_usage, page->frame, false);
            }
            
            log_trace(logger, "Page: %i | Frame: %i | P:%i | U:%i | M:%i", j, page->frame, page->P, page->U, page->M);

            page->frame = -1;
            page->P = 0;
            page->U = 0;
            page->M = 0;
            page->swap_pos = -1;

            log_trace(logger, "Libero PID: %i, Segmento: %i Pagina: %i", i, j, pcb->id);
        }

        log_info(logger, "Destruccion de Tabla de Paginas PID: %i - Segmento: %i - TAMAÑO: %i paginas", pcb->id, i, list_size(page_table));
    }

    send_process_finished_response(socket_kernel);
    log_debug(logger, "Se liberaron las paginas de PID: %i", pcb->id);
    free_pcb(pcb);
}

// analizamos si la tabla esta o no esta presente 
void resolve_page_fault()
{
	t_pcb* pcb = recv_pcb(socket_kernel);
    int segment = recv_int(socket_kernel);
    int page = recv_int(socket_kernel);

    log_debug(logger, "Comienzo de resolucion de Page Fault para PID: %i, Segment: %i, Page: %i", pcb->id, segment, page);

    usleep(memoria_config->swap_delay * 1000);

    t_page_table_data* page_data = get_page(pcb, segment, page);

    int frame;

    if(page_data->swap_pos == -1)
    {
        log_trace(logger, "No esta en disco");
        // No esta ni en disco
        frame = find_free_frame(pcb, segment, page);

        page_data->frame = frame;
        // Presencia en memoria Ram
        page_data->P = 1;
    }
    else
    {
        // Esta en disco
        // Traemos los datos de disco
        log_trace(logger, "Buscamos en disco");
        int* swap_data = read_page_from_swap(page_data, pcb, segment, page);

        frame = find_free_frame(pcb, segment, page);

        page_data->frame = frame;
        page_data->P = 1;
        page_data->M = 0;
        page_data->U = 0;

        pthread_mutex_lock(&ram_mutex);
        void* dest_ram = ram + memoria_config->page_size * frame;
        memcpy(dest_ram, swap_data, memoria_config->page_size * sizeof(int));
        free(swap_data);
        pthread_mutex_unlock(&ram_mutex);
    }

    send_tlb_consistency_check(socket_cpu_tlb, frame);
    send_page_fault_resolved(socket_kernel_page_fault, pcb);
    log_debug(logger, "Page Fault resuelto PID: %i | Segment: %i | Page: %i | Frame: %i", pcb->id, segment, page, frame);
    free_pcb(pcb);
}

int* read_page_from_swap(t_page_table_data* page_data, t_pcb* pcb, int segment, int page)
{
    swap = fopen(memoria_config->path_swap, "r");
    fseek(swap, page_data->swap_pos, SEEK_SET);
    log_trace(logger, "Nos posicionamos en %i", ftell(swap));
    int* page_swap = malloc(sizeof(int) * memoria_config->page_size);
    fread(page_swap, sizeof(int), memoria_config->page_size, swap);
    log_trace(logger, "Leemos hasta %i", ftell(swap));
    log_info(logger, "SWAP IN -  PID: %i - Marco: %i - Page In: %i|%i", pcb->id, page_data->frame, segment, page);
    fclose(swap);
    return page_swap;
}

void write_page_to_swap(t_page_table_data* page_data, t_pcb* pcb, int segment, int page)
{
    swap = fopen(memoria_config->path_swap, "a");
    void* ram_page_start = ram + page_data->frame * memoria_config->page_size;
    fwrite(ram_page_start, sizeof(int), memoria_config->page_size, swap);
    page_data->swap_pos = ftell(swap);
    log_info(logger, "SWAP OUT -  PID: %i - Marco: %i - Page Out: %i|%i", pcb->id, page_data->frame, segment, page);
    fclose(swap);
}

// Busca un frame libre, si no hay ninguno libre hace reemplazo
int find_free_frame(t_pcb* pcb, int segment, int page)
{
    int frame = -1;

    // Busca un frame libre
    for(int i = 0; i < list_size(frames_usage); i++)
    {
        if(!list_get(frames_usage, i)) 
        {
            log_trace(logger, "Encontre frame libre: %i", i);
            list_replace(frames_usage, i, true);
            frame = i;
            break;
        }
    }

    // No se encontro un frame libre, se hace reemplazo
    if(frame == -1)
    {
        log_trace(logger, "No hay frames libres");
        // La memoria esta llena
        t_page_table_data* victim = find_victim(pcb, segment, page);
        
        if(victim->M == 1)
        {
            // Escribir en disco
            write_page_to_swap(victim, pcb, segment, page);
        }

        victim->P = 0;
        frame = victim->frame;
    }

    t_page_table_data* page_data = get_page_reverse(pcb, frame);
    page_data->timestamp = global_time++;

    return frame;
}

// Devuelve una pagina victima para reemplazar
t_page_table_data* find_victim(t_pcb* pcb, int segment, int page)
{
    for(int o = 0; true; o++)
    {
        int page_tables_index = 0;
        int page_index = 0;
        log_trace(logger, "Buscando victima... iteracion: %i", o);
            
        t_last_page_table_data* last_page_table_data = list_get(last_page_table_reference, pcb->id);

        for(int i = last_page_table_data->last_page_table; i < list_size(page_tables); i++)
        {
            t_list* page_table = list_get(page_tables, i);
            //log_trace(logger, "Buscando en Segmento: %i", i);

            for(int j = last_page_table_data->last_page; j < list_size(page_table) - 1; j++)
            {
                t_page_table_data* page_data = list_get(page_table, j);
                t_page_table_data* page_data_next = list_get(page_table, j + 1);

                if(page_data->pid != pcb->id)
                    continue;

                if(page_data->timestamp < page_data_next->timestamp) {
                    page_tables_index = i;
                    page_index = j;
                }
                else {
                    page_tables_index = i;
                    page_index = j + 1;
                }
                
                // Puntero de la pagina
                last_page_table_data->last_page = (last_page_table_data->last_page + 1) % list_size(page_table);
            }
            // Puntero de la tabla de paginas
            last_page_table_data->last_page_table = (last_page_table_data->last_page_table + 1) % list_size(page_tables);
        }
        
        t_list* page_table = list_get(page_tables_index);
        t_page_table_data* page_data = list_get(page_table, page_index);

        if(is_victim(page_data, o))
        {
            log_debug(logger, "Se encontro victima!");
            log_info(logger, "REEMPLAZO - PID: %i - Marco: %i - Page Out: %i|%i - Page In: %i|%i", pcb->id, page_data->frame, page_tables_index, page_index, segment, page);
            return page_data;
        }
    }
}

bool is_victim(t_page_table_data* page, int iteration)
{
    page->timestamp = global_time++;
    if(strcmp(memoria_config->replace_algorithm, "CLOCK-M") == 0)
    {
        if(page->P == 0)
            return false;

        log_trace(logger, "Chequeo Frame:%i|P:%i|U:%i|M:%i", page->frame, page->P, page->U, page->M);

        if(iteration % 2 == 0)
        {
            if(page->U == 0 && page->M == 0)
            {
                return true;
            }
        }
        else
        {
            if(page->U == 0 && page->M == 1)
            {
                return true;
            }
            else
            {
                page->U = 0;
            }
        }
    }
    else
    // CLOCK NORMAL
    {
        if(page->P == 0)
            return false;

        log_trace(logger, "Chequeo Frame:%i|P:%i|U:%i|M:%i", page->frame, page->P, page->U, page->M);

        if(page->U == 0)
        {
            return true;
        }
        else
        {
            page->U = 0;
        }
    }
    return false;
}


// U = 1 y M = 0
// U = 1 y M = 1

// 1) busca U = 0 y M = 0
// si no encuentra
// 2) busca U = 0 y M = 1, aplicandole U = 0 
// 3) volver a 1)

// Aplica el algoritmo CLOCK o CLOCK MODIFICADO a una pagina. Devuelve:
// true -> si ya encontro la victima
// false -> si se tiene que seguir buscando