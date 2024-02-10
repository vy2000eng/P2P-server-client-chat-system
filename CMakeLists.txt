cmake_minimum_required(VERSION 3.27)
project(multithreaded_client_server_chat C)

set(CMAKE_C_STANDARD 11)

add_subdirectory(client)
add_subdirectory(server)
add_subdirectory(shared)

#add_executable(multithreaded_client_server_chat
#        client/client.c
#        client/client.h
#        client/main.c
#        client/main.h
#        server/main.c
#        server/main.h
#        server/server.c
#        server/server.h
#        shared/utils.c
#        shared/utils.h)
