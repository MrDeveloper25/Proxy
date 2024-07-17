cmake_minimum_required(VERSION 3.28)
project(Logs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(logs main.cpp
        logs.cpp
        logs.cpp)

target_link_libraries(logs ws2_32)

# MrDev25 - All Rights Reserved- 2024