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



int run_server();





#endif //M_T_C_S_CHAT_SERVER