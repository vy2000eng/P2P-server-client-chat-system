### Project Compilation Instructions

#### Client Compilation
gcc client/client.c client/main.c client/greeting.c client/messaging.c shared/utils.c -o client/cli -pthread

#### Server Compilation
gcc server/server.c server/main.c shared/utils.c -o server/ser -pthread

### /client
### General Overview
    The client component of this application is crafted to handle peer-to-peer (P2P) communications, connecting to a server to register its presence and then initiating or receiving connections for direct messaging between clients. 
    This setup enables clients to communicate directly with each other, bypassing the server after initial connection setup, thereby facilitating real-time message exchanges.

### Technical Overview
#### /client/client.h
    init_thread_args: 
    Prepares and initializes the client's configuration, including IP and port, based on command-line arguments. 
    This function is crucial for setting up the client's network parameters.

    run_client_server: 
    Sets up a listening socket for incoming P2P connections from other clients. 
    It dynamically allocates a port and listens for incoming connections, spawning a new thread for each P2P communication session initiated.

    establish_presence_with_server: 
    Connects to the central server to register the client's details, including its listening port. 
    This registration allows other clients to retrieve the necessary information to establish a direct P2P connection.

#### /client/messaging.h
    P2P_communication_thread Function: 
    This function initializes the P2P communication setup by creating and managing separate threads for sending and receiving messages. 
    It ensures synchronized access to shared resources using mutexes and handles thread lifecycle, including creation, execution, and cleanup.

    handle_sending Function: 
    Responsible for sending messages to a connected peer. It captures user input from the console and sends it to the peer using the send_packet function. 
    The loop continues until an exit condition is met, ensuring continuous message sending capability.

    handle_receiving Function: 
    Handles incoming messages from a connected peer. It continuously listens for messages using the receive_packet function and prints received messages to the console. 
    This loop ensures that the client is always ready to display new messages.

    

Function Descriptions:
```c
    int init_thread_args(thread_args **_thread_args, int argc, char **argv);
        //Initializes thread_args strings from command line arguments.

    void *run_client_server(void *arg);
        //Thread that listens for incoming connections on the client.

    connect_to_main_server();
        //Connects to the server's run_server() function, specifying the IP and port via program arguments (e.g., ./cli 127.0.0.1 3048).

    void *establish_presence_with_server(void *arg);
       //Logs the client for later retrieval by other clients.

    int connect_to_server(int *server_socket, char *ip, char *port);
        //Connects to a server given the IP and port.

    int initiate_P2P_connection(thread_args *_thread_args);
        //Initiates a P2P connection by requesting client information from the server.

    void clear_input_buffer();
        //Addresses buffer issues and ensures fgets blocks as expected.
```

## /server
### General Overview

    The server component of this application is designed to manage connections and communications between clients in a networked environment.
    It operates by setting up a listening socket that awaits incoming client connections. Once a client connects, the server initiates a separate
    thread to handle the communication with that client, ensuring concurrent handling of multiple clients without blocking the main server process.

### Technical Overview
#### /server/server.h
    set_up_server: 
    Initializes the server's listening socket, binds it to a specified port, and prepares it to listen for incoming connections. 
    It utilizes network programming practices to handle address reuse and bind operations, ensuring the server is ready to accept client connections.

    listen_for_connection: 
    Listens for incoming client connections. Upon a successful connection, it creates a new thread for each client, enabling simultaneous communications with multiple clients. 
    This function is crucial for maintaining a responsive server capable of handling multiple client interactions concurrently.

    connected_client_thread:
    A thread function dedicated to handling communication with a connected client. 
    It manages the reception of client information, such as usernames and ports, and processes client requests, whether for establishing a P2P connection or retrieving information about other clients.

    init_thread_args: 
    Prepares and initializes the arguments to be passed to the thread handling communication with a client.
    This setup is essential for passing client-specific data to the thread, ensuring that each thread has the necessary context to interact with its assigned client.

    set_client_address:
    Extracts and sets the client's IP address information. This function is key for identifying clients and facilitating their interactions within the network.

    print_client_info: 
    Utility function for logging client information, aiding in monitoring and debugging the server's operations.


#### Struct Definitions:
```c
typedef struct client_arr{
    client_info_packet client[128];
    int                size;
}clients_arr;
    
typedef struct server_thread_args{
    int                  socket;
    client_info_packet * client_info_packet_na;
    clients_arr        * connected_clients_arr;
}server_thread_args;
```

Function Descriptions:
```c
    void init_array(clients_arr *clientsArr);
        Initializes the client array.

    int run_server();
        Starts the server, listens for connections, and handles client threads.

    void *connected_client_thread(void *arg);
        Handles connected clients, receiving and storing their information.

    void set_client_address(int client_socket, client_info_packet *client_info_packet_incoming);
        Retrieves and sets the connected client's address information.
```

## /shared
### General Overview

    The shared utilities play a crucial role in facilitating communication and data exchange between the client and server components of this application. 
    Located in the shared directory, these utilities encompass a comprehensive suite of functions and data structures designed to standardize and streamline 
    the process of sending and receiving various types of packets over the network.
    
    The shared directory encapsulates all the necessary packets for client-server communication within this project.
    As the project evolves, the utilization and definition of packets are subject to updates. 
    Currently, the following packet types are actively used:

    packet_type
    base_packet
    username_packet
    client_info_packet
    action_packet
    message_packet

Further updates will include a comprehensive list of utilized packets as new implementations are introduced.

### Technical Overview


##### Function Descriptions:
    send_packet and receive_packet: High-level functions responsible for sending and receiving packets, respectively.
    They utilize the n_read and n_write utility functions to ensure that the entire packet is correctly transmitted over the network.

    n_read and n_write: Low-level functions designed to handle partial reads/writes due to network buffering. 
    They ensure the complete sending or receiving of a packet's data by repeatedly calling recv or send until all data is transferred.

##### Packet Description:
    packet_type:
        -   Defines the range of packet types utilized for client-server interactions, enabling streamlined packet processing and handling.
    
    base_packet:
        -   A foundational packet structure that includes essential information such as packet type and length.
            Every packet constructed in this project starts with a base_packet to ensure uniformity and predictability in data handling.
    
    username_packet:
        -   Incorporates a base_packet and specifies the username set by the client.
            This packet is sent to the server, which then transfers the username information into a client_info_packet for further processing.
    
    client_info_packet:
        -   Filled immediately upon the server's connection with a client, specifying the client's IP and port.
            The username is added subsequently after its transmission by the client.
            This packet is pivotal for enabling direct client-to-client connections and is initially stored in a client_arr structure.
            Future iterations may migrate this data to a more scalable storage solution, such as SQLite, along with additional identification methods.
    
    action_packet:
        -   Utilized for delineating various server actions based on the client's requests, such as establishing presence (value 0) or retrieving another client's information for a P2P connection (value 1).
   
    message_packet:
        -   The messaging system is designed around the message_packet structure, facilitating the transmission of text messages between clients.
            This system is composed of two primary components: the handle_sending and handle_receiving functions, each running in its own thread (sending_thread and receiving_thread) to manage message flow.

##### Packet Definitions

```c
    typedef enum
    {
        type_username_packet,
        type_connected_clients_packet,
        type_message_packet,
        type_port_packet,
        type_client_info_packet,
        type_action_packet
    } packet_type;

    typedef struct base_packet
    {
        ssize_t     length;
        packet_type type;
    }base_packet;

    typedef struct username_packet
    {
        base_packet packet_type;
        char        user_name[32];
    } __attribute__((packed)) username_packet;
    
    // connected_clients_packet isn't used.
    typedef struct
    {
        base_packet packet_type;
        char        connected_clients[4096];
    } connected_clients_packet;
    
    typedef struct message_packet
    {
        base_packet packet_type;
        char        message[256];
    } message_packet;
    
    typedef struct port_packet
    {
        base_packet packet_type;
        int         port;
    } __attribute__((packed)) port_packet;
 
    typedef  struct client_info_packet {
        base_packet packet_type;
        int         port;
        char        client_ip [INET_ADDRSTRLEN];
        char        username       [32];
    }  __attribute__((packed)) client_info_packet;
    
     typedef struct action{
        base_packet packet_type;
        int action;
    } __attribute__((packed)) action_packet;
```

