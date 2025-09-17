cmake_minimum_required(VERSION 3.28)
project(ex2)

set(CMAKE_CXX_STANDARD 17)

set(COMMON_FLAGS -fsanitize=thread -fsanitize=address)

add_executable(ex2 tests/main.cpp
        include/logger.h
        include/safe_queue.h
        include/logs_generator.h
        src/logs_generator.cpp
        tests/main.cpp
        include/logs_analyzer.h
        src/logs_analyzer.cpp
        src/safe_queue.cpp
        src/traffic.cpp
        include/traffic.h
        src/logger.cpp)