#include <math.h>
#include <stdbool.h>
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
extern t_memoria_config* memoria_config;
// Structures
extern void* ram;
extern FILE* swap;
extern t_list* page_tables;
extern t_list* frames_usage;

t_page_table_data* victim;

void handle_kernel()
{
    while(true) 
    {
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
        }
    }
}

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

            page->frame = -1;
            page->P = 0;
            page->U = 0;
            page->M = 0;
            page->swap_pos = -1;

            log_trace(logger, "Page: %i | Frame: %i | P:%i | U:%i | M:%i", j, page->frame, page->P, page->U, page->M);
 
            list_add(page_table, page);
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
}

void resolve_page_fault()
{
	t_pcb* pcb = recv_pcb(socket_kernel);
    int segment = recv_int(socket_kernel);
    int page = recv_int(socket_kernel);

    log_debug(logger, "Comienzo de resolucion de Page Fault para PID: %i, Segment: %i, Page: %i", pcb->id, segment, page);

	t_segment* segment_data = list_get(pcb->segment_table, segment);
    t_list* page_table = list_get(page_tables, segment_data->page_table_index);
    t_page_table_data* page_data = list_get(page_table, page);

    if(page_data->swap_pos == -1)
    {
        log_trace(logger, "No esta en disco");
        // No esta ni en disco
        int frame = find_free_frame(pcb, segment, page);

        page_data->frame = frame;
        page_data->P = 1;
    }
    else
    {
        // Esta en disco
        // Traemos los datos de disco
        log_trace(logger, "Buscamos en disco");
        void* swap_data = read_page_from_swap(page_data, pcb, segment, page);

        int frame = find_free_frame(pcb, segment, page);

        void* dest_ram = ram + memoria_config->page_size * frame;
        memcpy(dest_ram, swap_data, memoria_config->page_size * sizeof(int));
    }

    send_page_fault_resolved(socket_kernel);
    log_debug(logger, "Page Fault resuelto PID: %i, Segment: %i, Page: %i", pcb->id, segment, page);
}

void* read_page_from_swap(t_page_table_data* page_data, t_pcb* pcb, int segment, int page)
{
    swap = fopen(memoria_config->path_swap, "r");
    fseek(swap, page_data->swap_pos, SEEK_SET);
    void* page_swap = malloc(sizeof(int) * memoria_config->page_size);
    fread(&page_swap, sizeof(int), memoria_config->page_size, swap);
    log_info(logger, "SWAP IN -  PID: %i - Marco: %i - Page In: %i|%i", pcb->id, page_data->frame, segment, page);
    return page_swap;
}

void write_page_to_swap(t_page_table_data* page_data, t_pcb* pcb, int segment, int page)
{
    swap = fopen(memoria_config->path_swap, "a");
    void* ram_page_start = ram + page_data->frame * memoria_config->page_size;
    fwrite(ram_page_start, sizeof(int), memoria_config->page_size, swap);
    fclose(swap);
    page_data->swap_pos = ftell(swap);
    log_info(logger, "SWAP OUT -  PID: %i - Marco: %i - Page Out: %i|%i", pcb->id, page_data->frame, segment, page);
}

// Busca un frame libre, si no hay ninguno libre hace reemplazo
int find_free_frame(t_pcb* pcb, int segment, int page)
{
    int frame = -1;

    log_trace(logger, "Hay %i frame en el bitarray", list_size(frames_usage));

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

    return frame;
}

// Devuelve una pagina victima para reemplazar
t_page_table_data* find_victim(t_pcb* pcb, int segment, int page)
{
    for(int o = 0; true; o++)
    {
        log_trace(logger, "Buscando victima...");

        for(int i = 0; i < list_size(page_tables); i++)
        {
            t_list* page_table = list_get(page_tables, i);

            log_trace(logger, "Buscando en Segmento: %i", i);

            for(int j = 0; j < list_size(page_tables); j++)
            {
                t_page_table_data* page_data = list_get(page_tables, j);

                if(is_victim(page, o))
                {
                    log_debug(logger, "Se encontro victima!");
                    log_info(logger, "REEMPLAZO - PID: %i - Marco: %i - Page Out: %i|%i - Page In: %i|%i", pcb->id, page_data->frame, i, j, segment, page);
                    return page_data;
                }
            }
        }
    }
}

bool is_victim(t_page_table_data* page, int iteration)
{
    if(strcmp(memoria_config->replace_algorithm, "CLOCK-M") == 0)
    {
        if(page->P == 0)
            return false;

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
    {
        if(page->P == 0)
            return false;

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