cmake_minimum_required(VERSION 3.20)
project(logger)
set(CMAKE_CXX_STANDARD 20)


set(COMMON_FLAGS
        -Wall
        -Wextra
        -Wpedantic
        -Werror
)

add_library(liblogger
        include/logger.h
        src/logger.cpp)


add_executable(lab2_1 tests/test_logger.cpp)

target_link_libraries(lab2_1 liblogger)