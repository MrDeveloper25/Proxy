#ifndef PROXY_WEB_H
#define PROXY_WEB_H

#include <iostream>
#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define BUFFER_SIZE 4096
#define LOG_SERVER_IP  "127.0.0.1"
#define CENTRAL_SERVER_IP  "127.0.0.1"
#define LOG_SERVER_PORT 9090
#define CENTRAL_SERVER_PORT 9999


using namespace std;

void startProxyWeb(int proxy_port);
void handle_proxy_web(SOCKET client_socket, const char* log_server_ip, int log_server_port, const char* central_server_ip, int central_server_port);
void forward_data(SOCKET source_socket, SOCKET destination_socket);

#endif // PROXY_WEB_H

// MrDev25 - All Rights Reserved- 2024