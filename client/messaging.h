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
extern mtx_t termination_mutex;
extern mtx_t _mutex;

extern sem_t messaging_semaphore;
extern sem_t connection_semaphore;


extern int should_terminate;

typedef struct client_args{
    int  * connected_client_socket;
}client_args;

extern char user_input[256];

int         P2P_communication_thread(client_args * _client_args);
void *      handle_receiving(void * arg);
void *      handle_sending(void * arg);
void *      user_input_thread(void * args);
extern void clear_input_buffer();

#endif //MULTITHREADED_CLIENT_SERVER_CHAT_MESSAGING_H
