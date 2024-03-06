//
// Created by voldemort on 3/3/24.
//

#ifndef MULTITHREADED_CLIENT_SERVER_CHAT_MESSAGING_H
#define MULTITHREADED_CLIENT_SERVER_CHAT_MESSAGING_H


#include "../shared/utils.h"
#include "thread_db.h"
#include "threads.h"
#include "semaphore.h"


extern mtx_t communication_mutex;
extern sem_t messaging_semaphore;

typedef struct client_args{
    int  * connected_client_socket;
}client_args;

void * P2P_communication_thread(void * arg);
void * handle_receiving(void * arg);
void * handle_sending(void * arg);
extern void clear_input_buffer();




#endif //MULTITHREADED_CLIENT_SERVER_CHAT_MESSAGING_H
