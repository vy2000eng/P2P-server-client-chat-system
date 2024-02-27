
#include "main.h"


mtx_t client_arr_mutex;

int main(){

    mtx_init(&client_arr_mutex, mtx_plain);

    run_server();

    mtx_destroy(&client_arr_mutex);
    return 0;
}