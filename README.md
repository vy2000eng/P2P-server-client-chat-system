# Project Compilation Instructions


### Client Compilation
gcc client/client.c client/main.c client/greeting.c client/messaging.c shared/utils.c -o client/cli -pthread

### Server Compilation
gcc server/server.c server/main.c shared/utils.c -o server/ser -pthread

Client
General Overview

The client is responsible for three main functions:

    Establishing a listening socket for accepting other client connections.
    Connecting to the server and sending the port assigned to the listening socket.
    Optionally, connecting to another client or just listening for connections.

Technical Overview

    Semaphore and Mutex:
        extern sem_t packet_semaphore;
        extern mtx_t thread_args_mutex;

    Struct Definitions:
        typedef struct thread_args{
        char *ip;
        char *port;
        } thread_args;

Function Descriptions:

    int init_thread_args(thread_args **_thread_args, int argc, char **argv);
        Initializes thread_args strings from command line arguments.

    void *run_client_server(void *arg);
        Thread that listens for incoming connections on the client.

    connect_to_main_server();
        Connects to the server's run_server() function, specifying the IP and port via program arguments (e.g., ./cli 127.0.0.1 3048).

    void *establish_presence_with_server(void *arg);
        Logs the client for later retrieval by other clients.

    int connect_to_server(int *server_socket, char *ip, char *port);
        Connects to a server given the IP and port.

    int initiate_P2P_connection(thread_args *_thread_args);
        Initiates a P2P connection by requesting client information from the server.

    void clear_input_buffer();
        Addresses buffer issues and ensures fgets blocks as expected.

Server
General Overview

The server listens on a port for client connections, storing information necessary for clients to connect to each other. Future implementations will allow clients to request specific client information for direct connections.
Technical Overview

    Struct Definitions:
        typedef struct client_arr and typedef struct server_thread_args:
            Store client information and track the number of connected clients.

    Function Descriptions:

        void init_array(clients_arr *clientsArr);
            Initializes the client array.

        int run_server();
            Starts the server, listens for connections, and handles client threads.

        void *connected_client_thread(void *arg);
            Handles connected clients, receiving and storing their information.

        void set_client_address(int client_socket, client_info_packet *client_info_packet_incoming);
            Retrieves and sets the connected client's address information.

Shared
General Overview

The shared directory encapsulates all the necessary packets for client-server communication within this project. As the project evolves, the utilization and definition of packets are subject to updates. Currently, the following packet types are actively used:

    packet_type
    base_packet
    username_packet
    client_info_packet
    

Further updates will include a comprehensive list of utilized packets as new implementations are introduced.

Packet Definitions

    packet_type:
    Defines the range of packet types utilized for client-server interactions, enabling streamlined packet processing and handling.

    base_packet:
    A foundational packet structure that includes essential information such as packet type and length. 
    Every packet constructed in this project starts with a base_packet to ensure uniformity and predictability in data handling.
    
    username_packet: 
    Incorporates a base_packet and specifies the username set by the client. 
    This packet is sent to the server, which then transfers the username information into a client_info_packet for further processing.

    client_info_packet: 
    Filled immediately upon the server's connection with a client, specifying the client's IP and port. 
    The username is added subsequently after its transmission by the client. 
    This packet is pivotal for enabling direct client-to-client connections and is initially stored in a client_arr structure.
    Future iterations may migrate this data to a more scalable storage solution, such as SQLite, along with additional identification methods.
    
    action_packet: 
    Utilized for delineating various server actions based on the client's requests, such as establishing presence (value 0) or retrieving another client's information for a P2P connection (value 1).
    
    

