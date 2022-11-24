#include <math.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <shared/structures.h>
#include <shared/serialization.h>
#include "memoria.h"

extern t_log* logger;
extern int socket_kernel;
extern t_memoria_config* memoria_config;
// Structures
extern void* ram;
extern FILE* swap;
extern t_dictionary* page_tables_per_pid;

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
    t_list* page_tables = list_create();

    // Iteramos por cada segmento
    for(int i = 0; i < list_size(segments_sizes); i++)
    {
        int segment_size = list_get(segments_sizes, i);

        t_list* page_table = list_create();

        // Calculamos cuantas paginas necesitamos para el segmento
        int page_ammount = ceil(segment_size / memoria_config->page_size);
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

            list_add(page_table, page);
        }

        // Guardamos la tabla de pagina en nuestra lista de tablas de paginas
        list_add(page_tables, page_table);

        // Ya tenemos la tabla de paginas inicializada
        t_segment* segment = (t_segment*)malloc(sizeof(t_segment));
        segment->page_table_index = pid;
        segment->size = segment_size;

        // Guardamos este segmento en la tabla de segmentos
        list_add(segment_table, segment);
    }
    dictionary_put(page_tables_per_pid, string_itoa(pid), page_tables);

    send_segment_table(socket_kernel, segment_table);
}
//void send_process_finished(int socket, int pid, t_list* segments);
void process_finished()
{
    int pid = recv_int(socket_kernel);
    t_list* segment_table = recv_segment_table(socket_kernel);

    t_list* page_tables = (t_list*)dictionary_get(page_tables_per_pid, string_itoa(pid));

    for(int i = 0; i < list_size(page_tables); i++)
    {
        t_list* page_table = (t_list*)list_get(page_tables, i);

        for(int j = 0; j < list_size(page_table); j++)
        {
            t_page_table_data* page = (t_page_table_data*)list_get(page_table, j);

            page->frame = -1;
            page->P = 0;
            page->U = 0;
            page->M = 0;
            page->swap_pos = -1;

            log_trace(logger, "Libero Page: %i, PID: %i", j, pid);
        }
    }

    send_process_finished_response(socket_kernel);
}

void resolve_page_fault()
{
    send_page_fault_resolved(socket_kernel);
}