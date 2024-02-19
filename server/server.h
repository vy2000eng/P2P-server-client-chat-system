#ifndef M_T_C_S_CHAT_SERVER

#define M_T_C_S_CHAT_SERVER
#define MY_PORT "3048"
#define BACKLOG 10

#include "../shared/utils.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <thread_db.h>
#include <netdb.h>
#include <unistd.h>



typedef struct client_arr{
    // client      clients [128];
    client_info_packet client[128];
    int                size;
}clients_arr;

typedef struct server_thread_args{
    int socket;
    clients_arr  * connected_clients_arr;
}server_thread_args;

void init_array(clients_arr *clientsArr);

void print_client_info(client_info_packet *clientInfoPacket);

void * connected_client_thread(void * arg);

int run_server();

bool insert_client(client_info_packet * clientInfoPacket,clients_arr *clientsArr);

int init_thread_args(server_thread_args ** s_trd_args,clients_arr * connected_clients_arr, int socket_client );





#endif //M_T_C_S_CHAT_SERVER