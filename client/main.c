#include <stdio.h>
#include "main.h"



int main(int argc, char*argv[]){
    if(argc < 3)
    {
        fprintf(stderr,"usage: tcp_client hostname port\n" );
        return 1;
    }
    connect_to_server(argc, argv);
    return 0;
}