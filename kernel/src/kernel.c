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
#include <shared/socket.h>
#include <shared/serialization.h>
#include <commons/log.h>

t_log* logger;


int main(void) {

	logger = log_create("kernel.log", "kernel", true, LOG_LEVEL_TRACE);

	puts("Modulo Kernel!!!");

	int socket_server = start_server_module("KERNEL");

	int socket_client = accept(socket_server, NULL, NULL);

	char* msg = recv_msg(socket_client);

	log_trace(logger, "Msg: %s", msg);

}