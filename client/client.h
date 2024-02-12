#ifndef M_T_C_S_CHAT_CLIENT

#define M_T_C_S_CHAT_CLIENT
#define PORT "3048"

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


int connect_to_server(int argc, char ** argv);
int run_server();





#endif //M_T_C_S_CHAT_CLIENT