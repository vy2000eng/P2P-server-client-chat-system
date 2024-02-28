
#include "main.h"


mtx_t client_arr_mutex;

int main(){
    int listening_socket;

    mtx_init(&client_arr_mutex, mtx_plain);

    if(set_up_server(&listening_socket)< 0){
        perror("set_up_server(&listening_sockeet) failed.\n");
        return 1;

    }
    if(listen_for_connection(&listening_socket)< 0)
    {
        perror("accept() failed\n");
    }

    mtx_destroy(&client_arr_mutex);
    return 0;
}