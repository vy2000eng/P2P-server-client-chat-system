#include "utils.h"

void receive_packet(int socket_client, void * buf){
    base_packet header;
    n_read(socket_client, sizeof(base_packet), &header);

    header.type         = ntohl(header.type);
    header.length       = ntohl(header.length);
    ssize_t data_length = header.length - (long) sizeof (base_packet);
    switch (header.type) {
        case type_username_packet:
        {
            username_packet       *upacket;
            upacket             = (username_packet *)buf;
            upacket->packet_type =  header;
            n_read(socket_client, data_length, upacket->user_name);
        }
            break;
        case type_message_packet:
        {

            message_packet             *  m_packet;
            m_packet                   =  (message_packet*)buf;
            m_packet->packet_type       =  header;
            n_read(socket_client, sizeof(SOCKET), &(m_packet->recipient_socket));
            m_packet->recipient_socket =  ntohl(m_packet->recipient_socket);
            ssize_t remaining_length   =  data_length - sizeof(SOCKET);
            n_read(socket_client, remaining_length, m_packet->message);
        }
            break;
        case type_connected_clients_packet:
        {
            connected_clients_packet *  ccpacket;
            ccpacket                 =  (connected_clients_packet *)buf;
            ccpacket->packet_type     =  header;
            n_read(socket_client, data_length, ccpacket->connected_clients);
        }
            break;
//        case type_socket_fd_packet:
//        {
//            s_fd_packet            *      sfd_packet;
//            sfd_packet             =      (s_fd_packet *) buf;
//            sfd_packet->packet_type =      header;
//            n_read(socket_client, data_length, &(sfd_packet->recipient));
//            sfd_packet->recipient  = ntohl(sfd_packet->recipient);
//        }
//            break;
//        case type_client_info_packet:
//        {
//            ssize_t                                   remaining_length;
//            client_info_packet                 *      c_i_packet;
//            c_i_packet                         =           (client_info_packet*) buf;
//            c_i_packet->packet_type             =      header;
//            n_read(socket_client, sizeof(bool), &(c_i_packet->initiating_chat));
//            c_i_packet->initiating_chat        =      ntohl(c_i_packet->initiating_chat);
//            n_read(socket_client, sizeof(int), &(c_i_packet->listening_port));
//            c_i_packet->listening_port         =      ntohl(c_i_packet->listening_port);
//            n_read(socket_client, sizeof(int), &(c_i_packet->socket_file_descriptor));
//            c_i_packet->socket_file_descriptor =      ntohl(c_i_packet->socket_file_descriptor);
//            n_read(socket_client, sizeof(c_i_packet->username), c_i_packet->username);
//            n_read(socket_client, sizeof(c_i_packet->client_ip), &(c_i_packet->client_ip));
//        }
//            break;
    }
}

void send_packet(SOCKET socket_client, void * buf){
    base_packet *type = (base_packet*) buf;

    switch ( type->type) {
        case type_username_packet:
        {
            type->type = htonl(type->type);
            type->length = htonl(sizeof (username_packet));
            username_packet *upacket = (username_packet *)buf;
            printf("packet: %u\n", htonl(upacket->packet_type.length));

            n_write(socket_client, sizeof(username_packet), upacket);
        }
            break;

        case type_message_packet:
        {
            type->type = htonl(type->type);
            type->length = htonl(sizeof (message_packet));
            message_packet *m_packet = (message_packet*) buf;
            m_packet->recipient_socket = htonl(m_packet->recipient_socket);
            n_write(socket_client, sizeof(message_packet), m_packet);
        }
            break;

        case type_connected_clients_packet:
        {
            type->type = htonl(type->type);
            type->length = htonl(sizeof (connected_clients_packet));
            connected_clients_packet *ccpacket = (connected_clients_packet *)buf;
            n_write(socket_client, sizeof(connected_clients_packet), ccpacket);
        }
            break;

//        case type_socket_fd_packet:
//        {
//            type->type = htonl(type->type);
//            type->length = htonl(sizeof (s_fd_packet));
//            s_fd_packet * sfd_packet = (s_fd_packet*) buf;
//            sfd_packet->recipient = htonl( sfd_packet->recipient);
//            n_write(socket_client, sizeof(s_fd_packet), sfd_packet);
//        }
//            break;
//        case type_client_info_packet:
//        {
//            type ->type                        = htonl(type->type);
//            type->length                       = htonl(sizeof(client_info_packet));
//            client_info_packet * c_i_packet    = (client_info_packet*) buf;
//            c_i_packet->initiating_chat        = htonl(c_i_packet->initiating_chat);
//            c_i_packet->listening_port         = htonl(c_i_packet->listening_port);
//            c_i_packet->socket_file_descriptor = htonl(c_i_packet->socket_file_descriptor);
//            n_write(socket_client, sizeof(client_info_packet), c_i_packet);
//
//
//        }
//            break;
    }
}

static bool n_read(int fd, ssize_t len, void *buf) {

    //return false;
    int total = 0;
    int bytes_left = len;
    int n;
    while(total < len){                                  // reads data from fd, which is actually cli_sd
        n = recv(fd, buf + total, bytes_left,0);// with every iteration increments the spot to start reading from in the buf
        if(n <= 0)                                       // if there's an error than breaks out of loop
            break;
        total += n;
        bytes_left -= n;
    }
    return total == len? true : false;
}

/* attempts to write n bytes to fd; returns true on success and false on failure
It may need to call the system call "write" multiple times to reach the size len.
*/
static bool n_write(int fd, ssize_t len, void *buf) {
    int total = 0;
    int bytes_left = len;
    int n;
    while(total < len){                               //exactly the same functionality as n_read()
        n = send(fd, buf+total, bytes_left, 0);
        if(n <=0)
            break;
        total += n;
        bytes_left -= n;
    }
    return total == len? true:false;


}