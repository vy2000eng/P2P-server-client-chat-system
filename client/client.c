#include "client.h"

int connect_to_server(int argc, char ** argv){
    printf("printing args\n");
    printf("%s\n", argv[2]);

    struct      addrinfo        hints;
    struct      addrinfo        *res;
    int         gai_return;
    int         server_socket;

    memset(&hints, 0,sizeof hints);
    hints.ai_family     =  AF_UNSPEC;
    hints.ai_socktype   =  SOCK_STREAM;
    gai_return          =  getaddrinfo(argv[1],argv[2], &hints, &res);

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


    char buf[18];
    recv(server_socket,buf, sizeof buf, 0);
    printf("%s", buf);
    close(server_socket);

    return 0;
}