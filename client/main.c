#include "main.h"



int main(int argc, char*argv[]){
   // intro();

    client_info_packet client_info_packet_outgoing;

    if(argc < 3)
    {
        fprintf(stderr,"usage: tcp_client hostname port\n" );
        return 1;
    }
    run_client_server(&client_info_packet_outgoing);//these will be threads
    connect_to_main_server(argc, argv,&client_info_packet_outgoing);




    return 0;
}