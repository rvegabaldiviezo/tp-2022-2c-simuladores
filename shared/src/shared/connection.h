int SOCKET_KERNEL_CPU_DISPATCH;
int SOCKET_KERNEL_CPU_INTERRUPT;
int SOCKET_KERNEL_MEMORY;
int SOCKET_CPU_MEMORY;
int SOCKET_CPU_KERNEL;
int SOCKET_MEMORY_CPU;
int SOCKET_MEMORY_KERNEL;

int start_client(char* ip, char* port);
int start_server(char* ip, char* port);
void send_msg(char* msg, int socket);
char* recv_msg(int socket);