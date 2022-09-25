/*
 ============================================================================
 Name        : kernel.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <shared/connection.h>

int main(void) {

	int socket_server = start_server("0.0.0.0", "8000");

	puts("Esperando...");
	int socket_client = accept(socket_server, NULL, NULL);
	puts("Llego un cliente");

	char* msg = recv_msg(socket_client);
	printf("Recibi mensaje: %s", msg);
	free(msg);


	return EXIT_SUCCESS;
}
