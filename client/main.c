#include "main.h"




int main(int argc, char*argv[]) {
    // intro();
    thread_t      client_server_thread;
    void        * client_server_thread_return_value;
    thread_args * trd_args = NULL;
    char          initiate_connection;

    if (argc < 3)
    {
        fprintf(stderr, "usage: tcp_client hostname port\n");
        return 1;
    }
    should_terminate = 0;
    sem_init         (&packet_semaphore, 0,0);
    sem_init         (&connection_semaphore, 0,0);
    sem_init         (&messaging_semaphore, 0, 1);
    mtx_init         (&thread_args_mutex, mtx_plain);
    mtx_init         (&_mutex, mtx_plain);
    mtx_init         (&communication_mutex,mtx_recursive);
    mtx_init         (&termination_mutex, mtx_plain);
    init_thread_args (&trd_args, argc, argv);

    if(pthread_create(&client_server_thread, NULL,run_client_server,trd_args) != 0)
    {
        perror("Failed to create thread");
        return 1;
    }

    establish_presence_with_server(trd_args);

    do
    {
        printf("Initiate Connection With Another Client? (Y(Initiate Connection)/N(Listen For Connection)):");
        initiate_connection = getchar();
    }
    while(initiate_connection != 'Y' && initiate_connection !='y' && initiate_connection != 'N' && initiate_connection!='n');

    if(initiate_connection == 'Y' || initiate_connection == 'y')
    {
        if(initiate_P2P_connection(trd_args) < 0)
        {
            perror("initiating connection failed.\n");

        }
    }
    printf("after i_P2P_c\n");

    if(pthread_join(client_server_thread, &client_server_thread_return_value)!= 0 )
    {
        perror("Failed to join client server thread.\n");
        return 1;
    }
    printf("after join\n");

    if (client_server_thread_return_value  != NULL )
    {
        printf("Client server thread returned: %d\n", *(int*)client_server_thread_return_value);

        free(client_server_thread_return_value);
    }
    else
    {
        printf("Thread failed to return a value or allocate memory\n");
    }


    sem_destroy(&packet_semaphore);
    sem_destroy(&connection_semaphore);
    sem_destroy(&messaging_semaphore);
    mtx_destroy(&thread_args_mutex);
    mtx_destroy(&communication_mutex);
    free(trd_args->ip);
    free(trd_args->port);
    free(trd_args->listening_port);
    free(trd_args->username);
    free(trd_args);
    return 0;
}