
client compilation:
gcc client/client.c client/main.c client/greeting.c shared/utils.c -o client/cli -pthread

server compilation:
gcc server/server.c server/main.c shared/utils.c -o server/ser -pthread

************************************************************THE CLIENT**********************************************************************************
********************************************************************************************************************************************************
client:
general overview:
    -   The client is responsible for Three things:
        First, it establishes a listening socket that is used for accepting other client connections.
        Second, it establishes a connection with the server, and sends the port that was assigned to the listening socket.
        Third, if the client chooses, connect to another client, or just listen for connections. (this needs to be implemented).
    -   This is all run concurrently.

technical overview:
    extern sem_t packet_semaphore;
    extern mtx_t thread_args_mutex;

    typedef struct thread_args{
        char * ip;
        char * port;
    }thread_args;

    //initializes the strings inside of thread_args that are passed in via command line args
    int    init_thread_args(thread_args ** _thread_args,int argc, char ** argv);

    void * run_client_server   (void * arg);


    /*   - connect_to_main_server():
     *      -   Connects to the server in ../server/main.c "run_server()", on the port that server is listening on.
     *          For now, you need to specify it via program args ./cli 127.0.0.1 3048.
     *
     *      -   Leaves a record of the client in the server application of who exactly the client is.
     *          This is so that other clients can request it and use that information to connect to it later.
     */
    void * connect_to_main_server (void * arg);










************************************************************THE SERVER**********************************************************************************
********************************************************************************************************************************************************
server:
general overview
    -   The server is responsible for listening on a port and receiving a client connections.
        The server stores the client information necessary for a client to make a connection to another client.
        This is not implemented yet, but eventually a client will be able to request the information of a specific client,
        and connect to that client directly.

technical_overview:
typedef struct client_arr:
    - Stores client_info_packet.
    - keeps track of how many clients there are.

typedef struct server_thread_args:
    -   These are the argument that are passed into the void * connected_client_thread(void * arg);
    -   The socket, client_info_packet, and connected_client_arr get assigned in init_thread_args.

void init_array(clients_arr *clientsArr):
    -   Clears the array, and set the size to 0

int run_server():
    -   Initializes the client_arr, server_thread_args, and establishes the client address, and port.
    -   Listens for client connections and starts the connected_client_thread.
    -   The thread is detached because if it was waiting for it to finish; it wouldn't be able to accept another connection.

void * connected_client_thread(void * arg):
    -   Sends a message to the client confirming that the connection to the server has benn established.
    -   Receives the username of the client that is connected.
    -   Fills in the username for the client_info_packet, and inserts it into the connected_clients_arr.
    -   For right now, closes the socket, and frees the memory for the args.

void set_client_address(int client_socket, client_info_packet * client_info_packet_incoming):
    -   Sets the client address by filling out the sockaddr_in struct using the getpeername() function.
    -   The ip and port are retrieved using the sin_addr and sin_port attributes of the sockaddr_in struct,
        and are subsequently assigned and copied into the client_info_packet.port, and client_info_packet.client_ip struct.



typedef struct client_arr{
    client_info_packet client[128];
    int                size;
}clients_arr;

typedef struct server_thread_args{
    int socket;
    client_info_packet * client_info_packet_na;
    clients_arr        * connected_clients_arr;
}server_thread_args;

void init_array(clients_arr *clientsArr);

void print_client_info(client_info_packet *clientInfoPacket);

void * connected_client_thread(void * arg);

int run_server();

void set_client_address(int client_socket, client_info_packet * client_info_packet_incoming);

bool insert_client(client_info_packet * clientInfoPacket,clients_arr *clientsArr);

int init_thread_args(server_thread_args ** s_trd_args,clients_arr * connected_clients_arr, int socket_client, client_info_packet * client_info_packet_incoming );

***********************************************************SHARED***************************************************************************************
********************************************************************************************************************************************************

shared:
general overview:
    -   This directory has all the packets that are required for client server communication.
        There are currently more packets defined than are being used, but I'll update the comprehensive list of the packets being used as I make and use them.
        For right now the packet that are being used in this project are:
        packet_type,
        base_packet,
        username_packet,
        client_info_packet

technical overview:

packet_type:
    -   Even though this isn't really a packet, this is being used to determine the packet types, as stated previously the only ones being used
        at the time of this commit are username_packet and client_info_packet

base_packet:
    -   This packet is included in every packet that is constructed. It is used to determine what kind of packet is being sent or received, and the length in bytes of every packet.

username_packet:
    -   This packet includes the base_packet, and the username that is set by the client, and sent to the server.
        After the server receives this packet. The username for this packet is then copied into a client_info_packet.

client_info_packet:
    -   The port and client_ip are filled as soon as the server makes a connection with the client via get_peer_name().
        The username is later filled after the client sends it. This packet then gets stored into the client_arr struct, which for now allocates 128 instances of client_info_packet on the stack.
        This is just a preliminary version I eventually would like to refactor this into a sqllite db, and probably add some sort uuid.


typedef enum
{
    type_username_packet,
    type_connected_clients_packet,
    type_message_packet,
    type_client_info_packet
} packet_type;

typedef struct base_packet
{
    ssize_t     length;
    packet_type type;
}base_packet;


typedef struct username_packet
{
    base_packet packet_type;
    char        user_name[256];
} username_packet;


typedef  struct client_info_packet {
    base_packet packet_type;
    int         port;
    char        client_ip [INET_ADDRSTRLEN];
    char        username       [256];
}  __attribute__((packed)) client_info_packet;


