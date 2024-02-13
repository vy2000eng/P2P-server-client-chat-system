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
void init_array(clients_arr *clientsArr);

int run_server();

bool insert_client(client_info_packet * clientInfoPacket,clients_arr *clientsArr);





#endif //M_T_C_S_CHAT_SERVER