# Proxy System Installation and User Guide

## Introduction
This program is designed to run on Windows using Winsock. It consists of the following components:

- 2 Proxies
- 1 Centralized Server
- Load Balancer
- Web Filter
- Web Proxy
- Log Server

## Port Definitions
Each program component uses a specific port:

- Load Balancer: 3128
  ![load Balancer][loadBalancer.png]
- Web Filter: 8888 
  ![web Filter][Filter.png]
- Log Server: 9090
  ![log Server][logServer.png]
- Proxy: 8192
  ![Proxy][Proxy.png]
- Web Proxy: 8882
  ![WebProxy][webProxy.png]
- Centralized Server: 9999
  ![Centralized Server][Centralized_server.png]

## About the Project
This proxy system is programmed in C++. I have provided a CMake file to load all the necessary configurations. This is my first major project, and I'm excited to share it with the programming community.

I used CLion to develop this code, but if you use another IDE, please check the CMakeLists.txt file in each software module. For example, the Load Balancer has its own CMakeLists.txt file, and you might need to modify some settings to make the program work. If you use CLion, you only need to compile the main program, and everything else should work out of the box.

## Functionality
The client connects to a Load Balancer, which then connects to 2 proxies. These proxies send information to a Log Server. The proxies also connect to the internet. A centralized server is implemented to manage all connections on the correct ports.

## License
MrDev25 - All Rights Reserved - 2024

