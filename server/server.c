#include "server.h"
extern mtx_t client_arr_mutex;
// this is a temporary solution,just to get the main functionality working
void init_array(clients_arr *clientsArr){
    memset(clientsArr,0,sizeof (*clientsArr));
    clientsArr->size = 0;


}

// this is a temporary solution,just to get the main functionality working
bool insert_client(client_info_packet * clientInfoPacket,clients_arr *clientsArr){
    if(clientsArr->size > 127){
        return false;
    }else{
        clientsArr->client[clientsArr->size ++] = *clientInfoPacket;
        return true;

    }

}
int init_thread_args(server_thread_args ** s_trd_args,clients_arr * connected_clients_arr, int socket_client,client_info_packet * client_info_packet_incoming ){
    *s_trd_args    = malloc(sizeof (server_thread_args));
    if(*s_trd_args == NULL){ printf("Failed to allocate memory.\n"); return 1;}
    (*s_trd_args)->socket                   = socket_client;
    (*s_trd_args)->client_info_packet_na    = client_info_packet_incoming;
    (*s_trd_args)->connected_clients_arr    = connected_clients_arr;
    return 0;



}

void set_client_address(int client_socket_fd, client_info_packet * client_info_packet) {
    struct    sockaddr_in addr;
    socklen_t addr_size;
    char      client_ip[INET_ADDRSTRLEN]; // Buffer to store the IP address

    addr_size = sizeof(struct sockaddr_in);
    int res   = getpeername(client_socket_fd, (struct sockaddr *)&addr, &addr_size);

    if (res == 0)
    {
        // Successfully got the client address
        if (inet_ntop(AF_INET, &(addr.sin_addr), client_ip, INET_ADDRSTRLEN) != NULL)
        {
            memcpy(client_info_packet->client_ip, client_ip, sizeof (client_info_packet->client_ip));
        }
        else
        {
            perror("inet_ntop() failed");
        }
    }
    else
    {
        perror("getpeername failed");
    }
}


void * connected_client_thread(void * arg)
{
    server_thread_args                               *s_trd_args;
    username_packet                                  username_name_packet_incoming;
    port_packet                                      port_packet_incoming;
    port_packet_incoming.packet_type.type          = type_port_packet;
    username_name_packet_incoming.packet_type.type = type_username_packet;

    char client_connected_string[18]               = "client connected.\n";


    mtx_lock      (&client_arr_mutex);
    s_trd_args =  (server_thread_args*)arg;
    mtx_unlock    (&client_arr_mutex);



    send          (s_trd_args->socket, client_connected_string, sizeof(client_connected_string), 0);

    if(receive_packet(s_trd_args->socket, &port_packet_incoming)< 0)
    {perror("receive_packet(&port_packet_incoming) failed.");}

    if(receive_packet(s_trd_args->socket,&username_name_packet_incoming) < 0)
    {perror("receive_packet(&username_name_packet_incoming) failed.");}

    mtx_lock      (&client_arr_mutex);
    memcpy        (s_trd_args->client_info_packet_na->username, username_name_packet_incoming.user_name,sizeof (username_name_packet_incoming.user_name));
    s_trd_args->client_info_packet_na->port = port_packet_incoming.port;
    print_client_info(s_trd_args->client_info_packet_na);
    insert_client (s_trd_args->client_info_packet_na, s_trd_args->connected_clients_arr);
    mtx_unlock    (&client_arr_mutex);

    close         (s_trd_args->socket);
    free          (s_trd_args);
    s_trd_args = NULL;
    return NULL;
}


void print_client_info(client_info_packet * clientInfoPacket){
    printf("client username: %s", clientInfoPacket->username );
    printf("client ip: %s\n", clientInfoPacket->client_ip);
    printf("client port: %d\n\n", clientInfoPacket->port);
}

int set_up_server(int * listening_socket)
{
    //packets
    clients_arr          clients_arr;//eventually, this will be an sqllite db
    client_info_packet  client_info_packet_incoming;

  //  struct              sockaddr_in their_address;
    struct              addrinfo         hints;
    struct              addrinfo         *res;
    int                 gai_return;
  //  int                 listening_socket;
   // int                 socket_client;
    int                 enable;
   // int                 addr_len;

    client_info_packet_incoming.packet_type.type = type_client_info_packet;
    memset                                          (&hints, 0,sizeof hints);
    init_array                                      (&clients_arr);
    hints.ai_family                              =  AF_UNSPEC;
    hints.ai_socktype                            =  SOCK_STREAM;
    hints.ai_flags                               =  AI_PASSIVE;
    gai_return                                   =  getaddrinfo(NULL, MY_PORT, &hints, &res);
    enable                                       =  1;
  //  addr_len                                     =  sizeof (their_address);

    if(gai_return != 0)
    {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(gai_return));
        freeaddrinfo(res);
        return -1;
    }
    *listening_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    if(listening_socket < 0)
    {
        printf("socket() failed.\n");
        freeaddrinfo(res);
        return -1;
    }
    if (setsockopt(*listening_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        freeaddrinfo(res);
        return -1;
    }

    if(bind(*listening_socket, res->ai_addr, res->ai_addrlen) == -1)
    {
        perror("bind() failed");
        freeaddrinfo(res);
        return -1;
    }

    freeaddrinfo(res);
    if(listen(*listening_socket,BACKLOG ) < 0)
    {
       printf("listen() failed.");
       return -1;
    }

    //printf("...listening...\n");
//   while(1) {
//       server_thread_args * s_trd_args = NULL;
//       socket_client = accept(listening_socket, (struct sockaddr *) &their_address, (socklen_t *) &addr_len);
//       if (socket_client < 0) { break; };
//       // we have not yet assigned the port number yet. That happens in connected_client_thread
//       set_client_address(socket_client,&client_info_packet_incoming);
//
//       mtx_lock(&client_arr_mutex);
//       init_thread_args(&s_trd_args, &clients_arr, socket_client, &client_info_packet_incoming);
//       mtx_unlock(&client_arr_mutex);
//
//       thread_t client_thread;
//       pthread_create(&client_thread, NULL, connected_client_thread, s_trd_args);
//       pthread_detach(client_thread);
//
//    }
    return 1;
}

int listen_for_connection(const int * listening_socket){
    clients_arr         clients_arr;//eventually, this will be an sqllite db
    client_info_packet  client_info_packet_incoming;
    struct              sockaddr_in their_address;
    int                 socket_client;
    int                 addr_len;
    bool                accept_err;
    addr_len                 =  sizeof (their_address);
    accept_err = false;



    while(1) {
        server_thread_args * s_trd_args = NULL;
        socket_client = accept(*listening_socket, (struct sockaddr *) &their_address, (socklen_t *) &addr_len);
        if (socket_client < 0) { accept_err = true; break; };
        // we have not yet assigned the port number yet. That happens in connected_client_thread
        set_client_address(socket_client,&client_info_packet_incoming);

        mtx_lock(&client_arr_mutex);
        init_thread_args(&s_trd_args, &clients_arr, socket_client, &client_info_packet_incoming);
        mtx_unlock(&client_arr_mutex);

        thread_t client_thread;
        pthread_create(&client_thread, NULL, connected_client_thread, s_trd_args);
        pthread_detach(client_thread);

    }
    return accept_err ? -1:0;

}








