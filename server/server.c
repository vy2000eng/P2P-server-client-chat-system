#include "server.h"




// this is a temporary solution,just to get the main functionality working
void init_array(clients_arr *clientsArr){
    memset(clientsArr,0,sizeof (*clientsArr));
    clientsArr->size = 0;


}

// this is a temporary solution,just to get the main functionality working
bool insert_client(client_info_packet * clientInfoPacket,clients_arr *clientsArr){
    clientsArr->client[clientsArr->size ++] = *clientInfoPacket;
    return true;

}

int run_server()
{
    //packets
    clients_arr         clients_arr;//eventually, this will be an sqllite db
    username_packet     username_packet_incoming;
    client_info_packet  client_info_packet_incoming;

    struct              sockaddr_in their_address;
    struct              addrinfo         hints;
    struct              addrinfo         *res;
    int                 gai_return;
    int                 listening_socket;
    int                 socket_client;
    int                 enable;
    int                 addr_len;

    memset                 (&hints, 0,sizeof hints);
    init_array             (&clients_arr);
    hints.ai_family     =  AF_UNSPEC;
    hints.ai_socktype   =  SOCK_STREAM;
    hints.ai_flags      =  AI_PASSIVE;
    gai_return          =  getaddrinfo(NULL, MY_PORT, &hints, &res);
    enable              =  1;
    addr_len            =  sizeof (their_address);

    if(gai_return != 0)
    {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(gai_return));
        freeaddrinfo(res);
        return 0;
    }
    listening_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    if(listening_socket < 0)
    {
        printf("socket() failed.\n");
        freeaddrinfo(res);
        return 0;
    }
    if (setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        freeaddrinfo(res);
        return 0;
    }

    if(bind(listening_socket, res->ai_addr, res->ai_addrlen) == -1)
    {
        perror("bind() failed");
        freeaddrinfo(res);
        return 0;
    }

    freeaddrinfo(res);
    if(listen(listening_socket,BACKLOG ) < 0)
    {
       printf("listen() failed.");
    }

    printf("...listening...\n");
    while(1) {
        socket_client = accept(listening_socket, (struct sockaddr *) &their_address, (socklen_t *) &addr_len);
        if (socket_client < 0) { break; };
        char client_connected_string[18] = "client connected.\n";
        send(socket_client, client_connected_string, sizeof(client_connected_string), 0);
        receive_packet(socket_client,&username_packet_incoming);//getting the connected client information i.e. username,port,ip
        client_info_packet_incoming.packet_type.type      = type_client_info_packet;
        client_info_packet_incoming.socket_file_descriptor = socket_client;
        memcpy(client_info_packet_incoming.username, username_packet_incoming.user_name, sizeof(username_packet_incoming.user_name));


        close(socket_client);
    }
    return 1;




}








