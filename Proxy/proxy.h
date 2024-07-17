#ifndef PROXY_H
#define PROXY_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <thread>

#define PROXY_PORT 8192
#define BUFFER_SIZE 4096
#define LOG_SERVER_IP "127.0.0.1"
#define CENTRAL_SERVER_IP "127.0.0.1"
#define LOG_SERVER_PORT 9090
#define CENTRAL_SERVER_PORT 9999

using namespace std;

void startProxy();
void handle_proxy(SOCKET client_socket, const char* log_server_ip, int log_server_port, const char* central_server_ip, int central_server_port);
void forward(SOCKET source_socket, SOCKET destination_socket);

#endif // PROXY_H

// MrDev25 - All Rights Reserved- 2024