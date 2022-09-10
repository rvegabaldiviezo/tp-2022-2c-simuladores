#ifndef CLIENTESERVERA_H_
#define CLIENTESERVERA_H_

// Include de utils
//#include "../../src/utils/datos.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <assert.h>
#include <signal.h>
#include <netdb.h>
// Librerias para Hilos y Semaforos
#include <pthread.h>
#include <semaphore.h>
// Commons que permite la catedra
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>

//+++
void iniciar_modulo();
void iniciar_loggs();
void iniciar_configs();
void iniciar_estructuras();
void iniciar_semaforos();
void iniciar_conexiones();
void iniciar_server();

//+++
int recibir_operaciones_modulo();


//+++
void finalizar_modulo();


#endif /* CLIENTESERVERA_H_ */
