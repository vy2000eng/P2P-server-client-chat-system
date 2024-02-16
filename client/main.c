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

   pthread_create(&client_server_thread, NULL, run_client_server,trd_args );
   pthread_join(client_server_thread, NULL);

    free(trd_args);
    //run_client_server(&client_info_packet_outgoing);//these will be threads
    connect_to_main_server(argc, argv,&client_info_packet_outgoing);




    return 0;
}