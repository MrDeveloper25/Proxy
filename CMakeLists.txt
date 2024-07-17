cmake_minimum_required(VERSION 3.15)
project(Proxy)

set(CMAKE_CXX_STANDARD 14)


include_directories(${PROJECT_SOURCE_DIR})


add_executable(Proxy
        main.cpp
      	loadBalancer/loadBalancer.cpp
        webFilter/webFilter.cpp
        logs/logs.cpp
        proxy/proxy.cpp
        webProxy/webProxy.cpp
       
)


target_link_libraries(Proxy
        ws2_32
        ${PROJECT_SOURCE_DIR}/loadBalancer/curl/curl/lib/libcurl.a
        ${PROJECT_SOURCE_DIR}/loadBalancer/curl/curl/lib/libcurl.dll.a
        # Add more if its necessary
        -lkernel32 -luser32 -lgdi32 -lwinspool -lshell32 -lole32 -loleaut32 -luuid -lcomdlg32 -ladvapi32
)

# MrDev25 - All Rights Reserved- 2024