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

int main(void) {

	puts("Modulo Kernel!!!");

	int socket_server = start_server_module("KERNEL");
	int socket_to_cpu_dispatch = start_client_module("CPU_DISPATCH");
	int socket_to_cpu_interrupt = start_client_module("CPU_INTERRUPT");

}
