cmake_minimum_required(VERSION 3.28)
project(webProxy)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(proxy_web main.cpp
        webProxy.h
        webProxy.cpp
        webProxy.cpp)

target_link_libraries(webProxy ws2_32)
