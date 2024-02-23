
#include "main.h"


mtx_t client_arr_mutex;

int main(){

    mtx_init(&client_arr_mutex, mtx_plain);

    run_server();
    return 0;
}