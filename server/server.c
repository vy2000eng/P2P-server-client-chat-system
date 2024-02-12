#include "server.h"


int run_server()
{
    struct              sockaddr_storage their_address;
    struct              addrinfo         hints;
    struct              addrinfo         *res;
    socklen_t           addr_size;
    client_info_packet  client_info_packet_incoming;
    int                 gai_return;
    int                 listening_socket;
    int                 socket_client;
    int                 enable;

    memset(&hints, 0,sizeof hints);
    hints.ai_family     =  AF_UNSPEC;
    hints.ai_socktype   =  SOCK_STREAM;
    hints.ai_flags      =  AI_PASSIVE;
    gai_return          =  getaddrinfo(NULL, MY_PORT, &hints, &res);
    enable              =  1;

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
    while(1)
    {
       addr_size     = sizeof (their_address);
       socket_client = accept(listening_socket,(struct sockaddr*)&their_address,&addr_size);
       if(socket_client < 0){break;};
       char  client_connected_string [18]= "client connected.\n";
       send(socket_client, client_connected_string, sizeof (client_connected_string), 0);
       receive_packet(socket_client, &client_info_packet_incoming);//getting the connected client information i.e. username,port,ip
       close(socket_client);
    }
    return 1;




}








