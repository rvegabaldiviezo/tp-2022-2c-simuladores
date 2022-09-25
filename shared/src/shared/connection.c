#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<commons/log.h>

#include "connection.h"

int start_client(char* ip, char* port)
{
	struct addrinfo hints, *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	/*LLAMADA AL SISTEMA que devuelve informacion de RED sobre la IP
	 * y el PUERTO que le pasemos, en este caso del Servidor. Inyecta en
	 * variable server_info los datos necesarios para la creacion del Socket.*/
	getaddrinfo(ip, port, &hints, &server_info);

	// Ahora vamos a crear el socket Cliente.
	int socket_client = socket(server_info->ai_family,
                            server_info->ai_socktype,
						    server_info->ai_protocol);

	// Ahora que tenemos el socket, vamos a conectarlo.
	connect(socket_client, server_info->ai_addr, server_info->ai_addrlen);
	// nuestros procesos ya estan conectados

	freeaddrinfo(server_info);

	return socket_client;
}
int start_server(char* ip, char* port) {
	int socket_servidor; //Guarda el File Descriptor(IDs) representado por un entero.

	struct addrinfo hints, *servinfo; // Estruc q Contendra información sobre la dirección de un proveedor de servicios.

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	getaddrinfo(ip, port, &hints, &servinfo); //Traduce el nombre de una ubicación de servicio a un conjunto de direcciones de socket.

	// Creamos el socket de escucha del servidor
	socket_servidor = socket(servinfo->ai_family,
							 servinfo->ai_socktype,
							 servinfo->ai_protocol);

	/* bind() y listen() son las llamadas al sistema que realiza
	 * la preparacion por parte del proceso servidor */

	int activado = 1;
	setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	// 1) Asociamos el socket creado a un puerto
	bind(socket_servidor,servinfo->ai_addr, servinfo->ai_addrlen);

	// 2) Escuchamos las conexiones entrantes a ese socket, cuya unica responsabilidad
	// es notificar cuando un nuevo cliente este intentando conectarse.
	listen(socket_servidor,SOMAXCONN); // El servidor esta listo para recibir a los clientes (ESTA ESCUCHANDO).
	freeaddrinfo(servinfo);
	return socket_servidor;
}

void send_msg(char* msg, int socket)
{
    int size = sizeof(char) * 5;
	void* magic = malloc(size);
    memcpy(magic, msg, size);
    send(socket, msg, size, 0);
    free(magic);
}
char* recv_msg(int socket)
{
    char* msg = malloc(sizeof(char) * 5);
    recv(socket, msg, sizeof(char) * 5, 0);
    return msg;
}
