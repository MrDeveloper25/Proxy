// proxy.cpp
#include "proxy.h"

using namespace std;

void startProxy() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (result != 0) {
        cerr << "WSAStartup failed: " << result << "\n";
        return;
    }

    SOCKET proxy_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (proxy_socket == INVALID_SOCKET) {
        cerr << "Error creating proxy socket: " << WSAGetLastError() << "\n";
        WSACleanup();
        return;
    }

    sockaddr_in proxy_addr;
    proxy_addr.sin_family = AF_INET;
    proxy_addr.sin_port = htons(PROXY_PORT);
    proxy_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(proxy_socket, (sockaddr*)&proxy_addr, sizeof(proxy_addr)) == SOCKET_ERROR) {
        cerr << "Error binding proxy socket: " << WSAGetLastError() << "\n";
        closesocket(proxy_socket);
        WSACleanup();
        return;
    }

    if (listen(proxy_socket, 10) == SOCKET_ERROR) {
        cerr << "Error listening on proxy socket: " << WSAGetLastError() << "\n";
        closesocket(proxy_socket);
        WSACleanup();
        return;
    }

    cout << "Proxy server listening on port " << PROXY_PORT << "\n";

    while (true) {
        SOCKET client_socket = accept(proxy_socket, nullptr, nullptr);
        if (client_socket == INVALID_SOCKET) {
            cerr << "Error accepting connection: " << WSAGetLastError() << "\n";
            continue;
        }

        thread(handle_proxy, client_socket, LOG_SERVER_IP, LOG_SERVER_PORT, CENTRAL_SERVER_IP, CENTRAL_SERVER_PORT).detach();
    }

    closesocket(proxy_socket);
    WSACleanup();
}
void handle_proxy(SOCKET client_socket, const char* log_server_ip, int log_server_port, const char* central_server_ip, int central_server_port) {
    SOCKET log_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (log_socket == INVALID_SOCKET) {
        cerr << "Error creating log socket: " << WSAGetLastError() << "\n";
        closesocket(client_socket);
        return;
    }

    sockaddr_in log_addr;
    log_addr.sin_family = AF_INET;
    log_addr.sin_port = htons(LOG_SERVER_PORT);
    inet_pton(AF_INET, log_server_ip, &log_addr.sin_addr);

    if (connect(log_socket, (sockaddr*)&log_addr, sizeof(log_addr)) == SOCKET_ERROR) {
        cerr << "Error connecting to log server: " << WSAGetLastError() << "\n";
        closesocket(client_socket);
        closesocket(log_socket);
        return;
    }

    // Connection to the central server
    SOCKET central_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (central_socket == INVALID_SOCKET) {
        cerr << "Error creating central server socket: " << WSAGetLastError()  << "\n";
        closesocket(client_socket);
        closesocket(log_socket);
        return;
    }

    sockaddr_in central_addr;
    central_addr.sin_family = AF_INET;
    central_addr.sin_port = htons(CENTRAL_SERVER_PORT);
    inet_pton(AF_INET, central_server_ip, &central_addr.sin_addr);

    if (connect(central_socket, (sockaddr*)&central_addr, sizeof(central_addr)) == SOCKET_ERROR) {
        cerr << "Error connecting to central server: " << WSAGetLastError() << "\n";
        closesocket(client_socket);
        closesocket(log_socket);
        closesocket(central_socket);
        return;

    }

    thread client_to_log([client_socket, log_socket]() {
        forward(client_socket, log_socket);
    });

    thread log_to_client([client_socket, log_socket]() {
        forward(client_socket, log_socket);
    });

    client_to_log.join();
    log_to_client.join();

    closesocket(client_socket);
    closesocket(log_socket);
    closesocket(central_socket);
}

// MrDev25 - All Rights Reserved- 2024
