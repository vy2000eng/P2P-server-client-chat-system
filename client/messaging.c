//
// Created by voldemort on 3/3/24.
//

#include "messaging.h"




void * P2P_communication_thread(void * arg)
{
    client_args  * _client_args;
    int          * thread_return_value;
    void         * receiving_thread_return_value;
    void         * sending_thread_return_value;
    thread_t       receiving_thread;
    thread_t       sending_thread;


    mtx_lock(&communication_mutex);
    _client_args        = (client_args*)arg;
    mtx_unlock(&communication_mutex);

    thread_return_value = malloc(sizeof (int));

    printf("inside of P2P communication thread, server socket %d\n", *_client_args->connected_client_socket);

    if(pthread_create(&sending_thread, NULL, handle_sending, _client_args)!=0)
    {
        perror("Failed to create thread");
        *thread_return_value = -1;
        pthread_exit(thread_return_value);
    }
    if(pthread_create(&receiving_thread, NULL, handle_receiving, _client_args)!=0)
    {
        perror("Failed to create thread");
        *thread_return_value = -1;
        pthread_exit(thread_return_value);
    }

    if(pthread_join(sending_thread, &sending_thread_return_value)!= 0 )
    {
        *thread_return_value = -1;
        pthread_exit(thread_return_value);
    }
    if(pthread_join(receiving_thread, &receiving_thread_return_value)!= 0 )
    {
        *thread_return_value = -1;
        pthread_exit(thread_return_value);
    }
    free(_client_args);
    if (sending_thread_return_value  != NULL || receiving_thread_return_value != NULL)
    {
        printf("sending_thread_return_value thread returned: %d\n", *(int*)sending_thread_return_value);
        printf("receiving_thread_return_value thread returned: %d\n", *(int*)receiving_thread_return_value);
        free  (sending_thread_return_value);
        free  (receiving_thread_return_value);
    }
    else
    {
        printf("Thread failed to return a value or allocate memory\n");
    }

    *thread_return_value = 0;
    pthread_exit(thread_return_value);

}

void * handle_sending(void * arg)
{
    client_args  * _client_args;
    message_packet _message_packet;
    int          * thread_return_value;


    mtx_lock(&communication_mutex);
    _client_args = (client_args*)arg;
    mtx_unlock(&communication_mutex);

    memset(&_message_packet, 0, sizeof (message_packet));


    _message_packet.packet_type.type = type_message_packet;
    thread_return_value              = malloc(sizeof (int));

    while (1)
    {

     //   clear_input_buffer();
        fgets(_message_packet.message, sizeof (_message_packet.message), stdin);
        if(send_packet(*_client_args->connected_client_socket, &_message_packet) < 0)
        {
            perror("sending message failed.\n");
            *thread_return_value = -1;
            pthread_exit(thread_return_value);
        }
        printf("msg sent: %s\n", _message_packet.message);
        memset(&_message_packet, 0, sizeof (message_packet));

    }



}

void * handle_receiving(void * arg)
{
    client_args * _client_args;
    message_packet _message_packet;
    int          * thread_return_value;


    mtx_lock(&communication_mutex);
    _client_args = (client_args*)arg;
    mtx_unlock(&communication_mutex);

    memset(&_message_packet, 0, sizeof (message_packet));


    _message_packet.packet_type.type = type_message_packet;
    thread_return_value              = malloc(sizeof (int));


    while(1){
        if(receive_packet(*_client_args->connected_client_socket, &_message_packet) < 0)
        {
            perror("receiving message failed.\n");
            *thread_return_value = -1;
            pthread_exit(thread_return_value);
        }
        printf("msg received: %s\n",_message_packet.message);
        memset(&_message_packet, 0, sizeof (message_packet));

        // clear_input_buffer();
    }

}







