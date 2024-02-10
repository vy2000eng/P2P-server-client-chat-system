cmake_minimum_required(VERSION 3.27)
project(server)
set(CMAKE_C_STANDARD 11)
add_executable(ser main.c server.c ../shared/utils.c)