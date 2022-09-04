#ifndef DATOS_H_
#define DATOS_H_

// Include de utils
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

typedef enum {

	// OPERACIONES MODULO clienteA
	OPERACION_A = 200,
	OPERACION_B,

	// OPERACIONES MODULO clienteServerA
	OPERACION_C = 300,
	OPERACION_D,

	// OPERACIONES MODULO ServerA
	OPERACION_E = 400,
	OPERACION_F,

	ERROR = -1 //Ocurrio un error en la comunicacion
} operacion;


#endif /* DATOS_H_ */
