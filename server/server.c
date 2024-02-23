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
    (*s_trd_args) ->client_info_packet_na   = client_info_packet_incoming;
    (*s_trd_args)->connected_clients_arr    = connected_clients_arr;
    return 0;



}

void set_client_address(int client_socket_fd, client_info_packet * client_info_packet) {
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    char client_ip[INET_ADDRSTRLEN]; // Buffer to store the IP address

    int res = getpeername(client_socket_fd, (struct sockaddr *)&addr, &addr_size);
    if (res == 0) {
        // Successfully got the client address
        if (inet_ntop(AF_INET, &(addr.sin_addr), client_ip, INET_ADDRSTRLEN) != NULL) {
            client_info_packet->port = ntohs(addr.sin_port);
            printf("Client IP: %s, Port: %u\n", client_info_packet->client_ip, client_info_packet->port);
            memcpy(client_info_packet->client_ip, client_ip, sizeof (client_info_packet->client_ip));
        } else {
            perror("inet_ntop failed");
        }
    } else {
        perror("getpeername failed");
    }
}


void * connected_client_thread(void * arg){
    server_thread_args *s_trd_args;

    mtx_lock(&client_arr_mutex);
    s_trd_args = (server_thread_args*)arg;
    mtx_unlock(&client_arr_mutex);

    username_packet username_name_packet_incoming;
    char client_connected_string[18] = "client connected.\n";
    send(s_trd_args->socket, client_connected_string, sizeof(client_connected_string), 0);
    username_name_packet_incoming.packet_type.type        = type_client_info_packet;
    receive_packet(s_trd_args->socket,&username_name_packet_incoming);

    mtx_lock(&client_arr_mutex);
    memcpy(s_trd_args->client_info_packet_na->username, username_name_packet_incoming.user_name,sizeof (username_name_packet_incoming.user_name));
    insert_client(s_trd_args->client_info_packet_na, s_trd_args->connected_clients_arr);
    mtx_unlock(&client_arr_mutex);


    close(s_trd_args->socket);
    free(s_trd_args);
    return NULL;
}


void print_client_info(client_info_packet * clientInfoPacket){
    printf("client username: %s", clientInfoPacket->username );
    printf("client ip: %s\n", clientInfoPacket->client_ip);
    printf("client port: %d\n", clientInfoPacket->port);
}

int run_server()
{
    //packets
    clients_arr          clients_arr;//eventually, this will be an sqllite db
    username_packet      username_packet_incoming;
    client_info_packet  client_info_packet_incoming;

    struct              sockaddr_in their_address;
    struct              addrinfo         hints;
    struct              addrinfo         *res;
    int                 gai_return;
    int                 listening_socket;
    int                 socket_client;
    int                 enable;
    int                 addr_len;

    client_info_packet_incoming.packet_type.type = type_client_info_packet;
    memset                                          (&hints, 0,sizeof hints);
    init_array                                      (&clients_arr);
    hints.ai_family                              =  AF_UNSPEC;
    hints.ai_socktype                            =  SOCK_STREAM;
    hints.ai_flags                               =  AI_PASSIVE;
    gai_return                                   =  getaddrinfo(NULL, MY_PORT, &hints, &res);
    enable                                       =  1;
    addr_len                                     =  sizeof (their_address);

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
       server_thread_args * s_trd_args = NULL;
       socket_client = accept(listening_socket, (struct sockaddr *) &their_address, (socklen_t *) &addr_len);
       if (socket_client < 0) { break; };
       set_client_address(socket_client,&client_info_packet_incoming);

       mtx_lock(&client_arr_mutex);
       init_thread_args(&s_trd_args, &clients_arr, socket_client, &client_info_packet_incoming);
       mtx_unlock(&client_arr_mutex);

       thread_t client_thread;
       pthread_create(&client_thread, NULL, connected_client_thread, s_trd_args);
       pthread_detach(client_thread);
//        char client_connected_string[18] = "client connected.\n";
//        send(socket_client, client_connected_string, sizeof(client_connected_string), 0);
//        client_info_packet_incoming.packet_type.type        = type_client_info_packet;
//        receive_packet(socket_client,&client_info_packet_incoming);//getting the connected client information i.e. username,port,ip
//    insert_client(&client_info_packet_incoming, &clients_arr);


        //print_client_info(&client_info_packet_incoming);

       // close(socket_client);
    }
    return 1;




}









