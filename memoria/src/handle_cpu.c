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
	 *y luego recibir los datos con el formato correspondiente. El primer caso es el acceso es cuando la tlb del
	 *cpu hace TLB miss, por lo tanto hay que retornarle PID, SEG, PAG, FRAME para que lo guarde en su tlb. El otro
	 *tipo de acceso es cuando hay TLB Hit, donde se reciben los datos que tiene la tlb y se debe retornar el valor.
	 */
	log_trace(logger, "Envio a la CPU memory_size: %i y page_size: %i", memoria_config->memory_size, memoria_config->page_size);
	send_memdata(socket_cpu, memoria_config->memory_size, memoria_config->page_size);  //handshake con cpu
}