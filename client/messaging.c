//
// Created by voldemort on 3/3/24.
//

#include "messaging.h"


void * P2P_communication_thread(void * arg)
{
    client_args  * _client_args;
    int          * socket;
    int          * thread_return_value;
    void         * receiving_thread_return_value;
    void         * sending_thread_return_value;
    thread_t     receiving_thread;
    thread_t     sending_thread;

    _client_args = (client_args*)arg;
    thread_return_value = malloc(sizeof (int));


    if(pthread_create(&receiving_thread, NULL, handle_receiving, _client_args)!=0)
    {
            perror("Failed to create thread");
            *thread_return_value = -1;
            pthread_exit(thread_return_value);
    }

    if(pthread_create(&sending_thread, NULL, handle_sending, _client_args))
    {
            perror("Failed to create thread");
            *thread_return_value = -1;
            pthread_exit(thread_return_value);
    }
    if(pthread_join(receiving_thread, &receiving_thread_return_value)!= 0 )
    {
        *thread_return_value = -1;
        pthread_exit(thread_return_value);
    }
    if(pthread_join(sending_thread, &sending_thread_return_value)!= 0 )
    {
        *thread_return_value = -1;
        pthread_exit(thread_return_value);
    }

    if (sending_thread_return_value  != NULL || receiving_thread_return_value != NULL)
    {
        printf("sending_thread_return_value thread returned: %d\n", *(int*)sending_thread_return_value);
        printf("receiving_thread_return_value thread returned: %d\n", *(int*)receiving_thread_return_value);

        free(sending_thread_return_value);
        free(receiving_thread_return_value);
    } else {
        printf("Thread failed to return a value or allocate memory\n");
    }




}
