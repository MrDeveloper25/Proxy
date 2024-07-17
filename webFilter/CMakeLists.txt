cmake_minimum_required(VERSION 3.0)
project(webFilter)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Headers files
include_directories(${CMAKE_SOURCE_DIR})

# Files of the project
set(SOURCES
        main.cpp
       webFilter.cpp
)

# Generate exe
add_executable(webFilter ${SOURCES})

# Linking library ws2_32 (Winsock)
target_link_libraries(webFilter ws2_32)
