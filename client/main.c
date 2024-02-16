#include "main.h"



int main(int argc, char*argv[]){
   // intro();

    client_info_packet client_info_packet_outgoing;
    thread_t client_server_thread;
    thread_t main_server_thread;
    thread_args * trd_args;


    if(argc < 3)
    {
        fprintf(stderr,"usage: tcp_client hostname port\n" );
        return 1;
    }
    trd_args = malloc(sizeof (thread_args));
    memset(trd_args, 0 , sizeof (thread_args));
    trd_args->clientInfoPacket.packet_type.type = type_client_info_packet;
    trd_args->clientInfoPacket.port             = 10;
    memcpy(trd_args->clientInfoPacket.client_ip_port,"127.0.0.1", 16);

   // pthread_create(client_server_thread, NULL, ,trd_args )

    free(trd_args);
    run_client_server(&client_info_packet_outgoing);//these will be threads
    connect_to_main_server(argc, argv,&client_info_packet_outgoing);




    return 0;
}