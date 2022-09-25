#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include "serialization.h"

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
