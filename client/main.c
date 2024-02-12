#include "main.h"



int main(int argc, char*argv[]){
//  intro();
    char username_buffer[256];

    if(argc < 3)
    {
        fprintf(stderr,"usage: tcp_client hostname port\n" );
        return 1;
    }
    connect_to_server(argc, argv);
    printf("Enter Username: ");
    fgets(username_buffer, sizeof (username_buffer), stdin);



    return 0;
}