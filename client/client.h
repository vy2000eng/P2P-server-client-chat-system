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
#include <limits.h> // For INT_MAX and INT_MIN
#include <threads.h>
#include "semaphore.h"
#include "messaging.h"

extern sem_t packet_semaphore;
extern mtx_t thread_args_mutex;

typedef struct thread_args{
    char * ip;
    char * port;
    int  * listening_port;
}thread_args;

typedef struct client_args{
    int * connected_client_socket;
}client_args;

//initializes the strings inside of thread_args that are passed in via command line args
int    init_client_args    (client_args ** _client_args);
int    init_thread_args    (thread_args ** _thread_args,int argc, char ** argv);
void * run_client_server   (void * arg);
void   print_client_info(client_info_packet * clientInfoPacket);

/*   - establish_presence_with_server():
 *      -   Connects to the server in ../server/main.c "set_up_server()", on the port that server is listening on.
 *          For now, you need to specify it via program args ./cli 127.0.0.1 3048.
 *
 *      -   Leaves a record of the client in the server application of who exactly the client is.
 *          This is so that other clients can request it and use that information to connect to it later.
 */
int    establish_presence_with_server (thread_args * _thread_args);
void * request_client         (void * arg);
int    connect_to_server      (int * server_socket,  char * ip,char* port);
int    initiate_P2P_connection(thread_args * _thread_args);

void   clear_input_buffer();

//void check_buffer();




#endif //M_T_C_S_CHAT_CLIENT