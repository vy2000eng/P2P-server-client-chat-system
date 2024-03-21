#include "client.h"

pthread_mutex_t communication_mutex;
pthread_mutex_t termination_mutex;
sem_t packet_semaphore;
sem_t messaging_semaphore;
mtx_t thread_args_mutex;
//pthread_mutex_t _mutex;
sem_t connection_semaphore;
char user_input[256];
int should_terminate;


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
    (*_thread_args)   ->client_server_listening_socket   = (int  *) malloc(sizeof(int));

    (*_thread_args)   ->username   = NULL; // this null ptr is cuz we don't know the size, we'll allocate memory for this later;
    if((*_thread_args)->ip                             == NULL||
       (*_thread_args)->port                           == NULL||
       (*_thread_args)->client_server_listening_socket == NULL||
       (*_thread_args)->listening_port                 == NULL)
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
    thread_args     *   _thread_args;
    client_args     *   _client_args;
    int             *   thread_return_value;
    int                 accept_failure;
    int                 listening_socket;
    int                 socket_client;
    int                 enable;
    int                 addr_len;
    int                 gai_return;
    char                exit;

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
        perror("socket() failed.\n");
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

    if(listen(listening_socket,BACKLOG) < 0)
    {
        printf("listen() failed.");
        *thread_return_value = -1;
        pthread_exit(thread_return_value);
    }
    *_thread_args->client_server_listening_socket = listening_socket;
    sem_post(&packet_semaphore);

    while(1)
    {
        char                input;
        username_packet     username_packet_incoming;
        action_packet       action_packet_outgoing;
        memset(&username_packet_incoming, 0, sizeof (username_packet));
        memset(&action_packet_outgoing, 0, sizeof (action_packet));
        memset(&input, 0, sizeof (char ));

        input                                     = -1;
        username_packet_incoming.packet_type.type = type_username_packet;
        action_packet_outgoing.packet_type.type   = type_action_packet;
        socket_client                             = accept(listening_socket, (struct sockaddr*)&their_addr, (socklen_t*) &addr_len);


        if(socket_client < 0)
        {
            if (errno == EBADF || errno == EINVAL)
            { // Check specific error codes as needed
                printf("exiting because main thread shut down listening\n");
               accept_failure = 0;
               free(_client_args);
               break;
            }
            else
            {
                accept_failure = -1;
                free(_client_args);
                break;
            }
        }
        if(receive_packet(socket_client, &username_packet_incoming)<0)
        {
            perror("receive_packet(socket_client, &username_packet_incoming).\n");
            *thread_return_value = -1;
            pthread_exit(thread_return_value);
        }
        do
        {
            printf("accept incoming connection from: %s (y/n)?\n",username_packet_incoming.user_name );
            clear_input_buffer();
            input = getchar();
        }
        while(input != 'y' && input != 'Y' && input != 'n' && input != 'N');

        action_packet_outgoing.action = input == 'y' || input == 'Y'? 1:0;
        send_packet(socket_client,& action_packet_outgoing);

        if(htonl( action_packet_outgoing.action) == 1)
        {
            _client_args->connected_client_socket = &socket_client;
            clear_input_buffer();

            if(P2P_communication_thread(_client_args, 0) < 0)
            {
                perror("P2P communication function failed.\n");
                *thread_return_value = -1;
                return thread_return_value;
            }
            do
            {
                printf("continue listening (y(continue)/q(quit) ?  ");
                exit = getchar();
            }
            while(exit != 'y' && exit != 'Y' && exit != 'q' && exit != 'Q');
            if(exit == 'Q' || exit == 'q'){break;};

        }
    }

    //later there will be an exit condition which won't that is not a failure.
    printf("run client server ended.\n");
    *thread_return_value = accept_failure == -1? -1:0;
    return thread_return_value;
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
    // action 0 indicates establishing presence with the server
    action_packet_outgoing.action              =  0;
    port_packet_outgoing.port                  =  *_thread_args->listening_port;

    if(connect_to_server(&server_socket, _thread_args->ip, _thread_args->port) < 0)
    {
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

    if (_thread_args->username == NULL)
    {
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
    action_packet      action_packet_incoming;
    client_info_packet client_info_packet_incoming;
    username_packet    username_packet_outgoing_to_server;
    username_packet    username_packet_outgoing_to_client;
    client_args     *  _client_args;

    server_socket   = -1;
    P2P_socket      = -1;
    if (init_client_args(&_client_args) < 0)
    {
        perror("client_args initialization failed.");
    }     // freed inside of sending and receiving threads

    // this while loop is for asking the client to accept the connection if they fail
    while(1)
    {
        memset(&action_packet_outgoing, 0, sizeof (action_packet));
        memset(&action_packet_incoming, 0, sizeof (action_packet));
        memset(&client_info_packet_incoming, 0, sizeof (client_info_packet));
        memset(&username_packet_outgoing_to_server, 0, sizeof (username_packet));
        memset(&username_packet_outgoing_to_client, 0, sizeof (username_packet));


        // connect to the main server
        if (connect_to_server(&server_socket, _thread_args->ip, _thread_args->port) < 0)
        {
            perror("connect_to_server() failed.");
            return -1;
        }

        memset                                                (&username_packet_outgoing_to_server, 0, sizeof(username_packet));
        action_packet_outgoing.packet_type.type             = type_action_packet;
        client_info_packet_incoming.packet_type.type        = type_client_info_packet;
        username_packet_outgoing_to_server.packet_type.type = type_username_packet;
        username_packet_outgoing_to_client.packet_type.type = type_username_packet;
        action_packet_incoming.packet_type.type             = type_action_packet;
        //action 1 indicates that the client is retrieving a username.
        action_packet_outgoing.action = 1;

        // confirming the connection, I should get rid of this
        int res  = recv(server_socket, buf, sizeof(buf), 0);
        buf[res] = '\0'; // Ensure null-termination
        printf     ("%s\n", buf);


        if (send_packet(server_socket, &action_packet_outgoing) < 0)
        {
            perror("send_packet(server_socket, &action_packet_outgoing failed.\n");
            return -1;
        }

        printf("Enter Username of Client You Want To Chat With: ");

        clear_input_buffer();
        fgets(username_packet_outgoing_to_server.user_name, sizeof(username_packet_outgoing_to_server.user_name),stdin);
        username_packet_outgoing_to_server.user_name[strcspn(username_packet_outgoing_to_server.user_name,"\n")] = '\0';

        if (send_packet(server_socket, &username_packet_outgoing_to_server) < 0)
        {
            perror("send_packet() failed.");
            return -1;
        }

        if (receive_packet(server_socket, &client_info_packet_incoming) < 0)
        {
            perror("receive_packet(server_socket, &client_info_packet_incoming) failed.\n");
            return -1;
        }

        sprintf           (port_number, "%d", client_info_packet_incoming.port);
        print_client_info (&client_info_packet_incoming);
        close             (server_socket);
        server_socket  =  -1;


        // this is for establishing connection with the client i.e P2P socket
        if (connect_to_server(&P2P_socket, client_info_packet_incoming.client_ip, port_number) < 0)
        {
            perror("connect_to_server(&P2P_socket, client_info_packet_incoming.client_ip, port_number) failed\n");
            return -1;
        }

        _client_args->connected_client_socket = &P2P_socket;
        strcpy                                  (username_packet_outgoing_to_client.user_name, _thread_args->username);
        printf                                  ("...waiting for client to accept the connection...\n");

        if(send_packet(P2P_socket, &username_packet_outgoing_to_client) < 0)
        {
            perror("send_packet(P2P_socket, &username_packet_outgoing_to_client).\n");
            return -1;
        }

        if(receive_packet(P2P_socket, &action_packet_incoming)<0)
        {
            perror("send_packet(P2P_socket, &username_packet_outgoing_to_client).\n");
            return -1;
        }

        if (action_packet_incoming.action == 1)
        {
            printf("initiating connection with: %s\n",username_packet_outgoing_to_server.user_name);
            if(P2P_communication_thread(_client_args,1) < 0)
            {
                perror("P2P communication function failed.\n");
                return -1;
            }
            break;
        }
        else
        {
            char input = '\0';
            printf("The Client Refused The Connection.\n");
            do
            {
                printf("try connecting again? y(yes)/q(quit): \n");
                input = getchar();
            }
            while(input != 'y' && input != 'Y' &&input!= 'q' &&input!='Q');

            if(input == 'y' || input == 'Y'){ continue;};

            if( input == 'Q' || input== 'q')
            {
                free(_client_args);
                return -1;

            }
        }
    }//while(1)
    free(_client_args);


    return 0;
}

void print_client_info(client_info_packet * clientInfoPacket)
{
    printf("client username: %s\n", clientInfoPacket->username );
    printf("client ip: %s\n", clientInfoPacket->client_ip);
    printf("client port: %d\n", clientInfoPacket->port);
}

void clear_input_buffer()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { /* discard */ }

}





