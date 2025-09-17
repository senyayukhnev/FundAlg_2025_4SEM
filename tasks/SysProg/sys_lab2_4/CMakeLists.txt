cmake_minimum_required(VERSION 3.28)
project(logger_4)

set(CMAKE_CXX_STANDARD 17)

add_executable(ex4 main.cpp
        include/logger.h
        src/logger.cpp
        src/elevator.cpp)