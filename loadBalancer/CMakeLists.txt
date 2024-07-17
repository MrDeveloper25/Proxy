cmake_minimum_required(VERSION 3.28)
project(loadBalancer)

set(CMAKE_CXX_STANDARD 17)

# Headers of libcurl
include_directories("C:/Proxy/loadBalancer/curl/curl/include")

# Directories of libcurl
link_directories("C:Proxy/loadBalancer/curl/curl/lib")

# Files of the loadBalancer project
add_executable(balanceador_de_carga main.cpp
       	loadBalancer.h
        loadBalancer.cpp
       	loadBalancer.cpp)

# Target of the link libraries
target_link_libraries(loadBalancer libcurl ws2_32)

# MrDev25 - All Rights Reserved- 2024