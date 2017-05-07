cmake_minimum_required(VERSION 3.7)
set(CMAKE_C_STANDARD 11)

project(server)
set(SOURCE_FILES server.c)
add_executable(server ${SOURCE_FILES})

project(client)
set(SOURCE_FILES client.c)
add_executable(client ${SOURCE_FILES})