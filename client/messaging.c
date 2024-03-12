//
// Created by voldemort on 3/3/24.
//

#include "messaging.h"




int P2P_communication_thread(client_args * _client_args)
{
    thread_t        receiving_thread;
    thread_t        sending_thread;

    if(pthread_create(&sending_thread, NULL, handle_sending, _client_args)!=0)
    {
        perror("Failed to create thread");
        return -1;
    }

    if(pthread_create(&receiving_thread, NULL, handle_receiving, _client_args)!=0)
    {
        perror("Failed to create thread");
        return -1;
    }
    pthread_detach(receiving_thread);
    pthread_detach(sending_thread);

    return 0;

}

void * handle_sending(void * arg)
{
    client_args  * _client_args;
    message_packet _message_packet;

    mtx_lock(&communication_mutex);
    _client_args = (client_args*)arg;
    mtx_unlock(&communication_mutex);

    _message_packet.packet_type.type = type_message_packet;
    printf("TYPE YOU MSG AND PRESS ENTER\n");

    while (1)
    {
        fgets(_message_packet.message, sizeof (_message_packet.message), stdin);
        _message_packet.message[strcspn(_message_packet.message, "\n")] = '\0';

        if(send_packet(*_client_args->connected_client_socket, &_message_packet) < 0)
        {
            perror("send_packet(*_client_args->connected_client_socket, &_message_packet)\n");
            free  (_client_args);
            break;
        }
        printf("msg sent: %s\n", _message_packet.message);
        memset(&_message_packet, 0, sizeof (message_packet));
    }
    return NULL;
}

void * handle_receiving(void * arg)
{
    client_args * _client_args;
    message_packet _message_packet;

    mtx_lock       (&communication_mutex);
    _client_args = (client_args*)arg;
    mtx_unlock     (&communication_mutex);

    memset                             (&_message_packet, 0, sizeof (message_packet));
    _message_packet.packet_type.type = type_message_packet;

    while(1)
    {
        if(receive_packet(*_client_args->connected_client_socket, &_message_packet) < 0)
        {
            perror("receive_packet(*_client_args->connected_client_socket, &_message_packet\n");
            free  (_client_args);
            break;
        }
        printf("msg received: %s\n",_message_packet.message);
        memset(&_message_packet, 0, sizeof (message_packet));
    }
    return NULL;
}







