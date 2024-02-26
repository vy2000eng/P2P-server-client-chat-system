#ifndef M_T_C_S_CHAT_MAIN_UTILS

#define M_T_C_S_CHAT_MAIN_UTILS

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#define SOCKET int

typedef enum
{
    type_username_packet,
    type_connected_clients_packet,
    type_message_packet,
    type_port_packet,
    type_client_info_packet
} packet_type;

typedef struct base_packet
{
    ssize_t     length;
    packet_type type;
}base_packet;

typedef struct username_packet
{
    base_packet packet_type;
    char        user_name[32];
} __attribute__((packed)) username_packet;

typedef struct
{
    base_packet packet_type;
    char        connected_clients[4096];
} connected_clients_packet;

typedef struct message_packet
{
    base_packet packet_type;
    int         recipient_socket;
    char        message[1024];
} message_packet;

typedef struct port_packet
{
    base_packet packet_type;
    int         port;
} __attribute__((packed)) port_packet;

typedef  struct client_info_packet {
    base_packet packet_type;
    int         port;
    char        client_ip [INET_ADDRSTRLEN];
    char        username       [32];
}  __attribute__((packed)) client_info_packet;


void   receive_packet  (int socket_client, void *buf);
void   send_packet     (int socket_client, void *buf);
static bool n_read     (int socket_client, ssize_t len, void * buf);
static bool n_write    (int socket_client, ssize_t len, void * buf);








#endif //M_T_C_S_CHAT_MAIN_UTILS