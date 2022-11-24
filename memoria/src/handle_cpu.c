#include <commons/log.h>
#include <shared/structures.h>
#include <shared/serialization.h>
#include "memoria.h"

extern t_log* logger;
extern int socket_cpu;
extern t_memoria_config* memoria_config;

void* handle_cpu(void* arg)
{
	/*Interaccion memoria-cpu
	 *Hay dos tipos de acceso que puede hacer la cpu a memoria, por lo cual, habra que hacer recv de un op_code
	 *y luego recibir los datos con el formato correspondiente. El primer tipo de acceso es cuando hay TLB Hit, 
	 *donde se reciben los datos marco offset y posiblemente un valor dependiendo si es mov_in o mov_out (code 0:
	 *mov in, code 1: mov out). El otro caso es el acceso cuando la tlb del cpu hace TLB miss, por lo tanto hay que
	 *retornarle FRAME segun PID, SEG, PAG para que lo guarde en su tlb. 
	 */
	log_trace(logger, "Envio a la CPU memory_size: %i y page_size: %i", memoria_config->memory_size, memoria_config->page_size);
	send_memdata(socket_cpu, memoria_config->memory_size, memoria_config->page_size);  //handshake con cpu

	int counter = 0;
	while(true){

		int code = recv_request_code(socket_cpu);
		int frame,
			offset,
			pid,
			segment,
			page;
		uint32_t reg;

		switch(code){
		case 0:
			uint32_t value = 20; // valor random generico
			frame = recv_frame(socket_cpu);
			offset = recv_offset(socket_cpu);
			sleep(1);
			send_memory_value(socket_cpu, value);
			break;
		case 1:
			frame = recv_frame(socket_cpu);
			offset = recv_offset(socket_cpu);
			reg = recv_reg(socket_cpu);
			sleep(1);
			send_mov_out_ok(socket_cpu);
			break;
		case 2:
			counter++;
			if ((counter % 3) != 0){
				int code = 0; // 0 - existe, 1 - pf
				int frame = 25; // valor random generico
				send_mem_code(socket_cpu, code);
				send_frame(socket_cpu, frame);
			}
			else{
				int code = 1; // 0 - existe, 1 - pf
				send_mem_code(socket_cpu, code);
			}
			pid = recv_request_pid(socket_cpu);
			segment = recv_request_segment(socket_cpu);
			page = recv_request_page(socket_cpu);
			sleep(1);
			break;
		}
	}
}
