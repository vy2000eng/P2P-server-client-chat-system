#include "main.h"




int main(int argc, char*argv[]) {
    // intro();

    //client_info_packet client_info_packet_outgoing;
    thread_t client_server_thread;
    thread_t main_server_thread;
    thread_args *trd_args = NULL;


    if (argc < 3) {
        fprintf(stderr, "usage: tcp_client hostname port\n");
        return 1;
    }
    sem_init(&packet_semaphore, 0,0);
    mtx_init(&thread_args_mutex, mtx_plain);
    init_thread_args(&trd_args, argc, argv);

    pthread_create(&client_server_thread, NULL,run_client_server,trd_args);
    pthread_create(&main_server_thread, NULL,connect_to_main_server,trd_args);

 //   pthread_join(client_server_thread,NULL);
    pthread_join(main_server_thread,NULL);


    free(trd_args->ip);
    free(trd_args->port);
    free(trd_args);





    return 0;
}