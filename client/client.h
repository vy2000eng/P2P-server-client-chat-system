#ifndef M_T_C_S_CHAT_CLIENT

#define M_T_C_S_CHAT_CLIENT
#define PORT "3049"
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
#include <stdio.h>



/*
 *   - connect_to_main_server():
 *      -   Connects to the server in ../server/main.c "run_server()", on the port that server is listening on.
 *          For now, you need to specify it via program args ./cli 127.0.0.1 3048.
 *
 *      -   Leaves a record of the client in the server application of who exactly the client is.
 *          This is so that other clients can request it and use that information to connect to it later.
 *
 */
typedef struct thread_args{
    client_info_packet clientInfoPacket;
    char * ip;
    char * port;

}thread_args;

int init_thread_args(thread_args * _thread_args,int argc, char ** argv);

void * run_client_server   (void * arg);
void * connect_to_main_server (void * arg);






#endif //M_T_C_S_CHAT_CLIENT