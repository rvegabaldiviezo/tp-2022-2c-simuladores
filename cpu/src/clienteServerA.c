#include "clienteServerA.h"
#include "../../src/utils/datos.h"
//CONFIGS
t_log* logger;
t_config* config;

//MODULO
//t_modulo* modulo;



int main(int argc, char** argv) {

	iniciar_modulo();

	recibir_operaciones_modulo();

	finalizar_modulo();

	return EXIT_SUCCESS;
}

//++++++ PASOS PREVIOS +++++++++++++++++
void iniciar_modulo(){
	iniciar_loggs();
	iniciar_configs();
	iniciar_estructuras();
	iniciar_semaforos();
	iniciar_conexiones();
	iniciar_server();
}
void iniciar_loggs(){}
void iniciar_configs(){}
//Le asigna memoria a todas las estructuras que definamos en el .h
void iniciar_estructuras(){}
void iniciar_semaforos(){}
//Se conecta a todos los servers de los cuales es cliente.
void iniciar_conexiones(){}
//Levanta el server para poder tener clientes.
void iniciar_server(){}



//++++++++ ATIENDE PETICIONES Y LAS EJECUTA ++++++++
int recibir_operaciones_modulo(){
	while (1) {
		//log_info(logger, " EN ESPERA DE PETICIONES\n");
		operacion cod_op = OPERACION_B;//recibir_operacion(cpu->kernel_dispatch);

		switch (cod_op) {
			case OPERACION_A:
				//log_info(logger, " EN ESPERA DE PETICIONES\n");
				puts(" OPERACION A");
				break;
			case OPERACION_B:
				//log_info(logger, " EN ESPERA DE PETICIONES\n");
				puts(" OPERACION B");
				break;
			case ERROR:
				//log_error(logger, " DISPATCH, EL KERNEL SE DESCONECTO");
				puts(" OPERACION ERROR");
				return EXIT_FAILURE;
			default:
				//log_warning(logger," OPERACION DESCONOCIDA");
				break;
		}

		break;
	}
	return EXIT_SUCCESS;
}



//+++ LIBERAMOS TODA LA MEMORIA QUE PEDIMOS +++++++++
void finalizar_modulo(){
	puts(" FINALIZAMOS EL MODULO"); /* prints !!!Hello World!!! */
}


