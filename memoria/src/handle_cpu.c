#include <stdio.h>
#include <time.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <shared/structures.h>
#include <shared/serialization.h>
#include "memoria.h"

extern t_log* logger;
extern int socket_cpu;
extern t_memoria_config* memoria_config;
// Structures
extern void* ram;
extern FILE* swap;
extern t_list* page_tables;

// Mutex
extern pthread_mutex_t ram_mutex;

void* handle_cpu(void* arg)
{
	/*Interaccion memoria-cpu
	 *Hay dos tipos de acceso que puede hacer la cpu a memoria, por lo cual, habra que hacer recv de un op_code
	 *y luego recibir los datos con el formato correspondiente. El primer tipo de acceso es cuando hay TLB Hit, 
	 *donde se reciben los datos marco offset y posiblemente un valor dependiendo si es mov_in o mov_out (code 0:
	 *mov in, code 1: mov out). El otro caso es el acceso cuando la tlb del cpu hace TLB miss, por lo tanto hay que
	 *retornarle FRAME segun PID, SEG, PAG para que lo guarde en su tlb. 
	 */
	log_trace(logger, "Envio a la CPU inputs_table: %i y page_size: %i", memoria_config->inputs_table, memoria_config->page_size);
	send_memdata(socket_cpu, memoria_config->inputs_table, memoria_config->page_size);  //handshake con cpu

	while(true)
	{
		op_code op_code = recv_op_code(socket_cpu);

		switch (op_code)
		{
		case RAM_ACCESS_READ:
			ram_access_read();
			break;
		
		case RAM_ACCESS_WRITE:
			ram_access_write();
			break;

		case FRAME_ACCESS:
			frame_access();
			break;
		}
		
	}
}

void ram_access_read()
{
	t_pcb* pcb = recv_pcb(socket_cpu);
	int frame = recv_int(socket_cpu);
	int offset = recv_int(socket_cpu);

	usleep(memoria_config->memory_delay * 1000);
	
	t_page_table_data* page_data = get_page_reverse(pcb, frame);
	page_data->U = 1;

	int offset_ram = frame * memoria_config->page_size + offset;
	int value;

    pthread_mutex_lock(&ram_mutex);
	memcpy(&value, ram + offset_ram, sizeof(int));
    pthread_mutex_unlock(&ram_mutex);

	log_info(logger, "PID: %i - Acción: LEER - Dirección física: %i", pcb->id, offset_ram);
	log_debug(logger, "Lectura: %i", value);

	send_read_response(socket_cpu, value);
}
void ram_access_write()
{
	t_pcb* pcb = recv_pcb(socket_cpu);
	int frame = recv_int(socket_cpu);
	int offset = recv_int(socket_cpu);
	int value = recv_int(socket_cpu);

	usleep(memoria_config->memory_delay * 1000);

	t_page_table_data* page_data = get_page_reverse(pcb, frame);
	page_data->U = 1;
	page_data->M = 1;

	int offset_ram = frame * memoria_config->page_size + offset;

    pthread_mutex_lock(&ram_mutex);
	memcpy(ram + offset_ram, &value, sizeof(int));
    pthread_mutex_unlock(&ram_mutex);

	log_info(logger, "PID: %i - Acción: ESCRIBIR - Dirección física: %i", pcb->id, offset_ram);
	log_debug(logger, "Escritura: %i", value);

	send_write_response(socket_cpu);
}
void frame_access()
{
	t_pcb* pcb = recv_pcb(socket_cpu);
	int segment = recv_int(socket_cpu);
	int page = recv_int(socket_cpu);

	t_segment* segment_data = list_get(pcb->segment_table, segment);
	t_list* page_table = list_get(page_tables, segment_data->page_table_index);
	t_page_table_data* page_data = list_get(page_table, page);

	usleep(memoria_config->memory_delay * 1000);

	if(page_data->P == 1)
	{
		int frame = page_data->frame;
		log_info(logger, "Acceso a tabla de paginas PID: %i - Página: %i - Marco: %i", pcb->id, page, frame);
		send_frame_response(socket_cpu, frame);
	}
	else
	{
		log_debug(logger, "Page Fault, PID: %i - Página: %i", pcb->id, page);
		send_page_fault(socket_cpu);
	}
}
