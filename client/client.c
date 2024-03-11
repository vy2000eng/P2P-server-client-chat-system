#include "client.h"

mtx_t communication_mutex;
sem_t packet_semaphore;
sem_t messaging_semaphore;
mtx_t thread_args_mutex;
sem_t connection_semaphore;


int init_thread_args(thread_args ** _thread_args,int argc, char ** argv)
{
    size_t            len_of_argv_1;
    size_t            len_of_argv_2;
    len_of_argv_1   = strlen(argv[1]);
    len_of_argv_2   = strlen(argv[2]);
    printf            ("printing args\n");
    printf            ("ip: %s port: %s\n",argv[1], argv[2]);

    // you need to free ip and port before you free _thread args, or else you'll have a memory leak
    *_thread_args                        = malloc(sizeof(thread_args));
    if(*_thread_args == NULL){ printf("Failed to allocate memory.\n"); return -1;}
    (*_thread_args)   -> ip              = (char *) malloc(len_of_argv_1+1);
    (*_thread_args)   -> port            = (char *) malloc(len_of_argv_2+1);
    (*_thread_args)   ->listening_port   = (int  *) malloc(sizeof(int));
    (*_thread_args)   ->username         = NULL; // this null ptr is cuz we don't know the size, we'll allocate memory for this later;
    if((*_thread_args)->ip   == NULL  ||
       (*_thread_args)->port == NULL  ||
       (*_thread_args)->listening_port == NULL)
    {printf("Failed to allocate memory.\n"); return -1;};

    strcpy((*_thread_args)->ip,argv[1]);
    strcpy((*_thread_args)->port,argv[2]);

    return 0;
}

int init_client_args(client_args ** _client_args)
{
    *_client_args = malloc(sizeof(client_args));
    if(*_client_args == NULL){ printf("Failed to allocate memory.\n"); return -1;}
    return 0;
}


// listening on port 3049
void * run_client_server(void * arg){

    struct addrinfo     hints;
    struct addrinfo     *res;
    struct sockaddr     their_addr;
    struct sockaddr_in  sin;
    thread_t            P2P_thread;
    thread_args     *   _thread_args;
    client_args     *   _client_args;
//
    int             *   thread_return_value;
    int                 accept_failure;
    int                 listening_socket;
    int                 socket_client;
    int                 enable;
    int                 addr_len;
    int                 gai_return;

   // username_packet_incoming.packet_type.type = type_username_packet;
    memset                (&hints,0 ,sizeof hints);
    init_client_args      (&_client_args);
    hints.ai_family     = AF_UNSPEC;
    hints.ai_socktype   = SOCK_STREAM;
    hints.ai_flags      = AI_PASSIVE;
    _thread_args        = (thread_args*)arg;
    gai_return          = getaddrinfo(NULL, "0", &hints, &res);
    thread_return_value = malloc(sizeof (int));
    enable              = 1;
    accept_failure      = 0;

    if(gai_return != 0)
    {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(gai_return));
        freeaddrinfo(res);
        *thread_return_value = -1;
        pthread_exit(thread_return_value);
    }

    listening_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    if(listening_socket < 0)
    {
        printf("socket() failed.\n");
        freeaddrinfo(res);
        *thread_return_value = -1;
        pthread_exit(thread_return_value);
    }

    if (setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    {
        perror("setsockopt(SO_REUSEADDR) failed");
        freeaddrinfo(res);
        *thread_return_value = -1;
        pthread_exit(thread_return_value);
    }

    if(bind(listening_socket, res->ai_addr, res->ai_addrlen) == -1)
    {
        perror("bind() failed");
        freeaddrinfo(res);
        *thread_return_value = -1;
        pthread_exit(thread_return_value);
    }

    socklen_t len = sizeof(sin);
    if (getsockname(listening_socket, (struct sockaddr *)&sin, &len) == -1)
    {
        perror("getsockname() failed");
        *thread_return_value = -1;
        pthread_exit(thread_return_value);
    }
    else
    {
        printf("LISTENING PORT NUMBER: %d\n", ntohs(sin.sin_port));
        //this is muy importante
        *_thread_args->listening_port = ntohs(sin.sin_port);
    }
    freeaddrinfo(res);
    if(listen(listening_socket,BACKLOG ) < 0)
    {
        printf("listen() failed.");
        *thread_return_value = -1;
        pthread_exit(thread_return_value);
    }
    sem_post(&packet_semaphore);

    while(1)
    {
        char                input;
        username_packet     username_packet_incoming;
        action_packet       action_packet_outgoing;

        input                                     = -1;
        username_packet_incoming.packet_type.type = type_username_packet;
        action_packet_outgoing.packet_type.type   = type_action_packet;
        socket_client                             = accept(listening_socket, (struct sockaddr*)&their_addr, (socklen_t*) &addr_len);

        if(socket_client < 0)
        {accept_failure = -1; break;}
        receive_packet(socket_client, &username_packet_incoming);

        do
        {
            printf("accept incoming connection from: %s (y/n)?\n",username_packet_incoming.user_name );
            clear_input_buffer();
            input = getchar();
        }
        while(input != 'y' && input != 'Y' && input != 'n' && input != 'N');

        action_packet_outgoing.action = input == 'y' || input == 'Y'? 1:0;
        send_packet(socket_client,& action_packet_outgoing);

        if(htonl( action_packet_outgoing.action) == 1){
            _client_args->connected_client_socket = &socket_client;
            clear_input_buffer();
            pthread_create(&P2P_thread, NULL, P2P_communication_thread,_client_args );
            pthread_detach(P2P_thread);
        }
    }

    //later there will be an exit condition which won't that is not a failure.
    *thread_return_value = accept_failure == -1? -1:0;
    pthread_exit(thread_return_value);
}

int connect_to_server(int * server_socket, char * ip ,char *  port)
{
    struct              addrinfo hints;
    struct              addrinfo *res;
    int                 gai_return;

    memset                                        (&hints, 0,sizeof hints);
    hints.ai_family                            =  AF_UNSPEC;
    hints.ai_socktype                          =  SOCK_STREAM;
    gai_return = getaddrinfo(ip,port, &hints, &res);
    if(gai_return != 0)
    {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(gai_return));
        return -1;
    }
    *server_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if(server_socket< 0)
    {
        printf("socket() failed.\n");
        freeaddrinfo(res);
        return -1;
    }

    if(connect(*server_socket, res->ai_addr, res->ai_addrlen) < 0)
    {
        printf("bind() failed.\n");
        return -1;
    }
    freeaddrinfo(res);
    return 0;

}

int establish_presence_with_server(thread_args * _thread_args)
{
    sem_wait(&packet_semaphore);

    action_packet       action_packet_outgoing;
    port_packet         port_packet_outgoing;
    username_packet     username_packet_outgoing;
    size_t              username_len;
    char                buf[18];
    int                 server_socket;

    memset(&action_packet_outgoing, 0 , sizeof (action_packet));
    memset(&port_packet_outgoing, 0, sizeof(port_packet));
    memset(&username_packet_outgoing,  0,sizeof (username_packet));

    port_packet_outgoing.packet_type.type      =  type_port_packet;
    username_packet_outgoing.packet_type.type  =  type_username_packet;
    action_packet_outgoing.packet_type.type    =  type_action_packet;
    action_packet_outgoing.action              =  0;
    port_packet_outgoing.port                  =  *_thread_args->listening_port;

    if(connect_to_server(&server_socket, _thread_args->ip, _thread_args->port) < 0){
        perror("connect_to_server() failed.\n" );
        return -1;
    }

    // confirming connection by receiving "client connected." from server.
    int res = recv(server_socket, buf, sizeof (buf), 0);
    buf[res] = '\0'; // Ensure null-termination
    printf("%s\n", buf);

    if(send_packet (server_socket,&action_packet_outgoing)<0)
    { perror       ("send_packet() failed."); return -1;}

    if(send_packet (server_socket,&port_packet_outgoing) < 0)
    { perror       ("send_packet() failed."); return -1;}

    printf         ("Enter Username: ");
    fgets          (  username_packet_outgoing.user_name, sizeof (username_packet_outgoing.user_name), stdin);
    username_packet_outgoing.user_name[strcspn(username_packet_outgoing.user_name, "\n")] = '\0';

    username_len = strlen(username_packet_outgoing.user_name);
    _thread_args->username = malloc(username_len + 1);

    if (_thread_args->username == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1; // Exit if allocation fails
    }
    strcpy(_thread_args->username, username_packet_outgoing.user_name);


    if(send_packet (server_socket, &username_packet_outgoing) < 0 )
    { perror       ("send_packet() failed."); return -1;}

    close          (server_socket);
    return 0;
}

int initiate_P2P_connection(thread_args * _thread_args)
{
    int                server_socket;
    int                P2P_socket;
    char               buf[18];
    char               port_number[6];

    action_packet      action_packet_outgoing;
    action_packet       action_packet_incoming;
    client_info_packet client_info_packet_incoming;
    username_packet    username_packet_outgoing_to_server;
    username_packet    username_packet_outgoing_to_client;
    client_args     *  _client_args;
    thread_t           P2P_thread;

    server_socket   = -1;
    P2P_socket      = -1;

    if(connect_to_server(&server_socket, _thread_args->ip, _thread_args->port) < 0)
    {
        perror("connect_to_server() failed." );
        return -1;
    }

    memset          (&username_packet_outgoing_to_server, 0 , sizeof(username_packet));
    init_client_args(&_client_args); // freed inside P2P_communication_thread
    action_packet_outgoing.packet_type.type                = type_action_packet;
    client_info_packet_incoming.packet_type.type           = type_client_info_packet;
    username_packet_outgoing_to_server.packet_type.type    = type_username_packet;
    username_packet_outgoing_to_client .packet_type.type   = type_username_packet;
    action_packet_incoming.packet_type.type                = type_action_packet;
    action_packet_outgoing.action                          =  1;


    int res = recv(server_socket, buf, sizeof (buf), 0);
    buf[res] = '\0'; // Ensure null-termination
    printf("%s\n", buf);


    if(send_packet (server_socket,&action_packet_outgoing)<0)
    { perror       ("send_packet() failed."); return -1;}

    printf         ("Enter Username of Client You Want To Chat With: ");

    clear_input_buffer();
    fgets          (username_packet_outgoing_to_server.user_name, sizeof (username_packet_outgoing_to_server.user_name), stdin);
    username_packet_outgoing_to_server.user_name[strcspn(username_packet_outgoing_to_server.user_name, "\n")] = '\0';

    if(send_packet (server_socket, &username_packet_outgoing_to_server) < 0 )
    { perror       ("send_packet() failed."); return -1;}

    if(receive_packet(server_socket, &client_info_packet_incoming) < 0)
    { perror("receive_packet() failed."); return -1;}

    sprintf(port_number, "%d",client_info_packet_incoming.port);
    print_client_info(&client_info_packet_incoming );

    close(server_socket);

    server_socket = -1;

    connect_to_server(&P2P_socket,client_info_packet_incoming.client_ip, port_number);
    _client_args->connected_client_socket = &P2P_socket;

    strcpy        (username_packet_outgoing_to_client.user_name, _thread_args->username);
    send_packet   (P2P_socket, &username_packet_outgoing_to_client);
    receive_packet(P2P_socket,&action_packet_incoming);


    pthread_create(&P2P_thread, NULL, P2P_communication_thread,_client_args );
    pthread_detach(P2P_thread);
    return 0;
}

void print_client_info(client_info_packet * clientInfoPacket)
{
    printf("client username: %s", clientInfoPacket->username );
    printf("client ip: %s\n", clientInfoPacket->client_ip);
    printf("client port: %d\n\n", clientInfoPacket->port);
}

void clear_input_buffer()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { /* discard */ }

}





