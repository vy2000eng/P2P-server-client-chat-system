#include "client.h"


sem_t packet_semaphore;
mtx_t thread_args_mutex;
int init_thread_args(thread_args ** _thread_args,int argc, char ** argv){
    size_t            len_of_argv_1;
    size_t            len_of_argv_2;
    len_of_argv_1   = strlen(argv[1]);
    len_of_argv_2   = strlen(argv[2]);
    printf            ("printing args\n");
    printf            ("ip: %s port: %s\n",argv[1], argv[2]);

    // you need to free ip and port before you free _thread args, or else you'll have a memory leak
    *_thread_args                        = malloc(sizeof(thread_args));
    if(*_thread_args == NULL){ printf("Failed to allocate memory.\n"); return 1;}
    (*_thread_args)   -> ip              = (char *) malloc(len_of_argv_1+1);
    (*_thread_args)   -> port            = (char *) malloc(len_of_argv_2+1);
    (*_thread_args)   ->listening_port   = (int  *) malloc(sizeof(int));
    if((*_thread_args)->ip   == NULL  ||
       (*_thread_args)->port == NULL  ||
       (*_thread_args)->listening_port == NULL)
    {printf("Failed to allocate memory.\n"); return 1;};

    strcpy((*_thread_args)->ip,argv[1]);
    strcpy((*_thread_args)->port,argv[2]);

    return 0;
}


// listening on port 3049
void * run_client_server(void * arg){
    struct addrinfo hints;
    struct addrinfo *res;
    struct sockaddr their_addr;
    struct sockaddr_in sin;
    thread_args     *_thread_args;
    int             gai;
    int             listening_socket;
    int             socket_client;
    int             enable;
    int             addr_len;
    int             gai_return;


    memset            (&hints,0 ,sizeof hints);
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;
    _thread_args      = (thread_args*)arg;


    gai_return        = getaddrinfo(NULL, "0", &hints, &res);
    enable            = 1;
    if(gai_return != 0){
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(gai_return));
        freeaddrinfo(res);
        //return 0;
    }

    listening_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    if(listening_socket < 0)
    {
        printf("socket() failed.\n");
        freeaddrinfo(res);
        //return 0;
    }

    if (setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        freeaddrinfo(res);
        //return 0;
    }

    if(bind(listening_socket, res->ai_addr, res->ai_addrlen) == -1)
    {
        perror("bind() failed");
        freeaddrinfo(res);
       // return 0;
    }

    socklen_t len = sizeof(sin);
    if (getsockname(listening_socket, (struct sockaddr *)&sin, &len) == -1) {
        perror("getsockname");
    } else {
        printf("Port number %d\n", ntohs(sin.sin_port));
        *_thread_args->listening_port = ntohs(sin.sin_port);
    }



    freeaddrinfo(res);
    if(listen(listening_socket,BACKLOG ) < 0)
    {
        printf("listen() failed.");
    }
   // get_port_and_ip(listening_socket, _thread_args);
    sem_post(&packet_semaphore);

    while(1){
        socket_client = accept(listening_socket, (struct sockaddr*)&their_addr, (socklen_t*) &addr_len);
        if(socket_client < 0){break;}
        char client_connected_string[29] = "client connected to client.\n";
        send(socket_client, client_connected_string, sizeof(client_connected_string), 0);
    }
    //return 1;
}

void *  connect_to_main_server(void * arg){

    port_packet         port_packet_outgoing;
    username_packet     username_packet_outgoing;
    thread_args        *_thread_args;
    char               buf[17];
    struct             addrinfo hints;
    struct             addrinfo *res;
    int                gai_return;
    int                server_socket;

    sem_wait (&packet_semaphore);
    memset   (&hints, 0,sizeof hints);
    hints.ai_family                            =  AF_UNSPEC;
    hints.ai_socktype                          =  SOCK_STREAM;
    port_packet_outgoing.packet_type.type      = type_port_packet;
    username_packet_outgoing.packet_type.type  = type_username_packet;


   _thread_args  = (thread_args*)arg;

   // this gai is for connecting to the server
    gai_return    = getaddrinfo(_thread_args->ip,_thread_args->port, &hints, &res);

    if(gai_return != 0)
    {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(gai_return));
        exit(1);
    }

    server_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    if(server_socket< 0)
    {
        printf("socket() failed.\n");
        freeaddrinfo(res);
    }

    if(connect(server_socket, res->ai_addr, res->ai_addrlen) < 0)
    {
        printf("bind() failed.\n");
    }
    freeaddrinfo(res);
    // confirming connection by receiving "client connected." from server.
    recv        (server_socket,buf, sizeof buf, 0);
    printf      ("%s", buf);
    port_packet_outgoing.port = *_thread_args->listening_port;
    //memcpy      (port_packet_outgoing.port, (*_thread_args).listening_port, sizeof (port_packet_outgoing.port));
    send_packet (server_socket,&port_packet_outgoing);

    printf      ("Enter Username: ");
    fgets       (  username_packet_outgoing.user_name, sizeof (username_packet_outgoing.user_name), stdin);

    send_packet (server_socket, &username_packet_outgoing);
    close       (server_socket);

    return 0;
}




