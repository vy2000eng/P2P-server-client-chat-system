#include "main.h"




int main(int argc, char*argv[]) {
    // intro();
    thread_t      client_server_thread;
    char          initiate_connection;
    char          main_loop;
    void        * client_server_thread_return_value;
    thread_args * trd_args;
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    // char

    if (argc < 3)
    {
        fprintf(stderr, "usage: tcp_client hostname port\n");
        return 1;
    }

    trd_args         = NULL;
    should_terminate =  0;
    sem_init(&packet_semaphore, 0,0);
    sem_init(&connection_semaphore, 0,0);
    sem_init(&messaging_semaphore, 0, 1);

    pthread_mutex_init  (&communication_mutex,NULL);
    pthread_mutex_init  (&termination_mutex, NULL);
    init_thread_args    (&trd_args, argc, argv);

    if(pthread_create(&client_server_thread, NULL,run_client_server,trd_args) != 0)
    {
        perror("Failed to create thread");
        return 1;
    }

    establish_presence_with_server(trd_args);

    while(1)
    {
        do
        {
            printf("Initiate Connection With Another Client? (Y(Initiate Connection)/N(Listen For Connection)):");
            initiate_connection = getchar();
        }
        while
                (initiate_connection != 'Y' &&
                 initiate_connection != 'y' &&
                 initiate_connection != 'N' &&
                 initiate_connection != 'n');


        if (initiate_connection == 'Y' || initiate_connection == 'y')
        {
            if (initiate_P2P_connection(trd_args) < 0)
            {
                perror("initiating connection failed.\n");
            }
        }
        else
        {
            printf("...listening for connections incoming...\n");
        }

        printf("after i_P2P_c\n");
        printf("after i_P2P_c\n");

        if (initiate_connection == 'y')
        {
            shutdown(*trd_args->client_server_listening_socket, SHUT_RDWR);
        }
        if (pthread_join(client_server_thread, &client_server_thread_return_value) != 0)
        {
            perror("Failed to join client server thread.\n");
            return 1;
        }
        printf("after join\n");

        if (client_server_thread_return_value != NULL)
        {
            printf("Client server thread returned: %d\n", *(int *) client_server_thread_return_value);
            free(client_server_thread_return_value);
        }
        else
        {
            printf("Thread failed to return a value or allocate memory\n");
        }
        do
        {
            printf("continue(y/n)?");
            main_loop = getchar();
        }
        while(main_loop != 'Y' &&
              main_loop != 'y' &&
              main_loop != 'N' &&
              main_loop != 'n');

        if(main_loop == 'n' || main_loop == 'N')break;

    }




    sem_destroy(&packet_semaphore);
    sem_destroy(&connection_semaphore);
    sem_destroy(&messaging_semaphore);
    pthread_mutex_destroy(&communication_mutex);
    pthread_mutex_destroy(&communication_mutex);

    free(trd_args->ip);
    free(trd_args->port);
    free(trd_args->listening_port);
    free(trd_args->username);
    free(trd_args->client_server_listening_socket);
    free(trd_args);
    return 0;
}