cmake_minimum_required(VERSION 3.27)
project(client)
set(CMAKE_C_STANDARD 11)
add_executable(cli main.c client.c ../shared/utils.c
        greeting.c
        greeting.h)
