cmake_minimum_required(VERSION 3.28)
project(sys_prog C)

set(CMAKE_C_STANDARD 11)

add_executable(sys_prog lab1_1.c)
add_executable(lab1_7 lab1_7.c)
add_executable(lab1_3 lab1_3.c)
add_executable(lab1_5 lab1_5.c)
add_executable(lab1_1 lab1_1.c)
add_executable(lab1_2 lab1_2/lab1_2.c)

add_executable(server_4 lab1_4/server.c)
target_link_libraries(server_4)
add_executable(client_4 lab1_4/client.c)
target_link_libraries(client_4)
target_sources(server_4 PRIVATE lab1_4/common.h)
target_sources(client_4 PRIVATE lab1_4/common.h)
add_executable(server_6
        lab1_6/server.c
)

add_executable(client_6
        lab1_6/client.c
)
target_sources(server_6 PRIVATE lab1_6/common.h)
target_sources(client_6 PRIVATE lab1_6/common.h)
