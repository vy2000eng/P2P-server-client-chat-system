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
    *_thread_args                                       = malloc(sizeof(thread_args));
    if(*_thread_args == NULL){ printf("Failed to allocate memory.\n"); return 1;}
    (*_thread_args)-> ip                                = (char *) malloc(len_of_argv_1+1);
    (*_thread_args)-> port                              = (char *) malloc(len_of_argv_2+1);
    if((*_thread_args)->ip   == NULL ||
       (*_thread_args)->port == NULL)
    {printf("Failed to allocate memory.\n"); return 1;};

    strcpy((*_thread_args)->ip,argv[1]);
    strcpy((*_thread_args)->port,argv[2]);

    return 0;
}
void get_port_and_ip(int listening_socket, thread_args * _thread_args){
    char ip[INET_ADDRSTRLEN];
  //  char portstr[6];
    // char ipstr[INET6_ADDRSTRLEN];
    struct sockaddr_storage ss; // Use sockaddr_storage to accommodate both IPv4 and IPv6
    socklen_t len = sizeof(ss);
    if (getsockname(listening_socket, (struct sockaddr *)&ss, &len) == -1) {
        perror("getsockname failed");
        // Handle error
    }

// Check if it's IPv4 or IPv6 and handle accordingly
    if (ss.ss_family == AF_INET) {
//        if (getnameinfo((struct sockaddr *)&ss, len, ip, sizeof ip, portstr, sizeof portstr, NI_NUMERICHOST | NI_NUMERICSERV) != 0) {
//            perror("getnameinfo");
//            exit(EXIT_FAILURE);
//        }
//
//        printf("Listening on IP: %s, Port: %s\n", ip, portstr);
        // It's IPv4

        struct sockaddr_in *s = (struct sockaddr_in *)&ss;
        inet_ntop(AF_INET, &s->sin_addr, ip, sizeof(ip)); // Convert IP to string
        int my_port = ntohs(s->sin_port); // Convert network byte order to host byte order
        _thread_args->client_info_packet_outgoing.port = my_port;
        memcpy(_thread_args->client_info_packet_outgoing.client_ip_port, ip, sizeof(ip));
    } else if (ss.ss_family == AF_INET6) {
        // It's IPv6
        struct sockaddr_in6 *s = (struct sockaddr_in6 *)&ss;
        inet_ntop(AF_INET6, &s->sin6_addr, ip, sizeof(ip)); // Convert IP to string
        int my_port = ntohs(s->sin6_port); // Convert network byte order to host byte order
        _thread_args->client_info_packet_outgoing.port = my_port;
        memcpy(_thread_args->client_info_packet_outgoing.client_ip_port, ip, sizeof(ip));
    } else {
        // Handle unexpected address family
        fprintf(stderr, "Unknown address family");
    }

}


// listening on port 3049
void * run_client_server(void * arg){
    struct addrinfo hints;
    struct addrinfo *res;
    struct sockaddr their_addr;
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


    gai_return        = getaddrinfo("127.0.0.1", "0", &hints, &res);
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

    freeaddrinfo(res);
    if(listen(listening_socket,BACKLOG ) < 0)
    {
        printf("listen() failed.");
    }
    get_port_and_ip(listening_socket, _thread_args);
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
//    printf("printing args\n");
//    printf("%s\n", argv[2]);
    //username_packet    username_packet_outgoing;
    client_info_packet  client_info_packet_outgoing;
    thread_args        *_thread_args;
    char               *username_buffer;
    char               buf[18];
    struct             addrinfo hints;
    struct             addrinfo *res;
    int                gai_return;
    int                server_socket;
    int                port;
    size_t             length_of_username;
    memset(&hints, 0,sizeof hints);
    hints.ai_family     =  AF_UNSPEC;
    hints.ai_socktype   =  SOCK_STREAM;

    sem_wait(&packet_semaphore);
    _thread_args                                   = (thread_args*)arg;
    gai_return = getaddrinfo(_thread_args->ip,_thread_args->port, &hints, &res);

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
    recv(server_socket,buf, sizeof buf, 0);
    printf("%s\n", buf);

    printf("Enter Username: ");

    _thread_args->client_info_packet_outgoing.packet_type.type   = type_client_info_packet;
    _thread_args->client_info_packet_outgoing.packet_type.length = sizeof (client_info_packet);
    fgets       (  _thread_args->client_info_packet_outgoing.username, sizeof (_thread_args->client_info_packet_outgoing.username), stdin);
    printf("packet ready");
    send_packet (server_socket, &_thread_args->client_info_packet_outgoing);
    close       (server_socket);

    return 0;
}


int char_to_int( char * string){
    char* endPtr;
    errno = 0; // To detect overflow

    long num = strtol(string, &endPtr, 10);

    // Check for various errors
    if (endPtr == string) {
        printf("No digits were found\n");
    } else if (errno == ERANGE || num > INT_MAX || num < INT_MIN) {
        printf("The number is out of range for an int\n");
    } else {
        return (int)num; // Cast is safe here
        //printf("The number is %d\n", intNum);
    }
    return -1;

}


