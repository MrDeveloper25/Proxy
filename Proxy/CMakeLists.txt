cmake_minimum_required(VERSION 3.0)
project(Proxy)

set(CMAKE_CXX_STANDARD 11)

add_executable(Proxy
        main.cpp
        proxy.cpp
        loadBalancer/loadBalancer.cpp
        webFilter/webFilter.cpp
        logs/logs.cpp
        proxy/proxy.cpp
       	webProxy/webProxy.cpp
)

target_link_libraries(Proxy PRIVATE ws2_32)
