cmake_minimum_required(VERSION 3.12)
project(Progetto_so_Final C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")

add_executable(Client Client/Client.c)
add_executable(Server Server/Server.c)