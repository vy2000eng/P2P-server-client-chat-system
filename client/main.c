#include "main.h"




int main(int argc, char*argv[]) {
    // intro();


    thread_t      client_server_thread;
   // thread_t      main_server_thread;
    void        * client_server_thread_return_value;
  //  void        * main_server_thread_return_value;
    thread_args * trd_args = NULL;

    if (argc < 3) {
        fprintf(stderr, "usage: tcp_client hostname port\n");
        return 1;
    }
    sem_init        (&packet_semaphore, 0,0);
    mtx_init        (&thread_args_mutex, mtx_plain);
    init_thread_args(&trd_args, argc, argv);

    if(pthread_create(&client_server_thread, NULL,run_client_server,trd_args) != 0)
    {
        perror("Failed to create thread");
        return 1;
    }
    connect_to_main_server(trd_args);

//    if(pthread_create(&main_server_thread, NULL,connect_to_main_server,trd_args) !=0)
//    {
//        perror("Failed to create thread");
//        return 1;
//    }





//    if(pthread_join(main_server_thread,&main_server_thread_return_value) != 0 ){
//
//        perror("Failed to join main server thread.\n");
//        return 1;
//    }

    if(pthread_join(client_server_thread, &client_server_thread_return_value)!= 0 ){

        perror("Failed to join client server thread.\n");
        return 1;

    }


    if (client_server_thread_return_value  != NULL )
    {
       // printf("Main Server Thread returned: %d\n", *(int*)main_server_thread_return_value);
        printf("Client server thread returned: %d\n", *(int*)client_server_thread_return_value);

        free(client_server_thread_return_value);
       // free(main_server_thread_return_value);
    } else {
        printf("Thread failed to return a value or allocate memory\n");
    }


    sem_destroy(&packet_semaphore);
    free(trd_args->ip);
    free(trd_args->port);
    free(trd_args->listening_port);
    free(trd_args);
    return 0;
}