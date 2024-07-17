#include "webProxy.h"


void startProxyWeb(int proxy_port) {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
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
    proxy_addr.sin_port = htons(proxy_port);
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

    cout << "Proxy server listening on port " << proxy_port << "\n";

    while (true) {
        SOCKET client_socket = accept(proxy_socket, nullptr, nullptr);
        if (client_socket == INVALID_SOCKET) {
            cerr << "Error accepting connection: " << WSAGetLastError() << "\n";
            continue;
        }

        thread(handle_proxy_web, client_socket, LOG_SERVER_IP, LOG_SERVER_PORT, CENTRAL_SERVER_IP, CENTRAL_SERVER_PORT).detach();
    }

    closesocket(proxy_socket);
    WSACleanup();

}

void handle_proxy_web(SOCKET client_socket, const char* log_server_ip, int log_server_port, const char* central_server_ip, int central_server_port) {
    // Connect to the local log server
    SOCKET log_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (log_socket == INVALID_SOCKET) {
        cerr << "Error creating server socket: " << WSAGetLastError() << "\n";
        closesocket(client_socket);
        return;
    }

    sockaddr_in log_addr;
    log_addr.sin_family = AF_INET;
    log_addr.sin_port = htons(log_server_port);
    inet_pton(AF_INET, log_server_ip, &log_addr.sin_addr);

    if (connect(log_socket, (sockaddr*)&log_addr, sizeof(log_addr)) == SOCKET_ERROR) {
        cerr << "Error connecting to web server: " << WSAGetLastError() << "\n";
        closesocket(client_socket);
        closesocket(log_socket);
        return;
    }

    // Connect to the central server
    SOCKET central_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (central_socket == INVALID_SOCKET) {
        cerr << "Error creating central server socket: " << WSAGetLastError() << "\n";
        closesocket(client_socket);
        closesocket(log_socket);
        return;
    }

    sockaddr_in central_addr;
    central_addr.sin_family = AF_INET;
    central_addr.sin_port = htons(central_server_port);
    inet_pton(AF_INET, central_server_ip, &central_addr.sin_addr);

    if (connect(central_socket, (sockaddr*)&central_addr, sizeof(central_addr)) == SOCKET_ERROR) {
        cerr << "Error connecting to central server: " << WSAGetLastError() << "\n";
        closesocket(client_socket);
        closesocket(log_socket);
        closesocket(central_socket);
        return;
    }

    // Forward data between client and log server
    thread client_to_log([client_socket, log_socket]() {
        forward_data(client_socket, log_socket);
    });

    // Forward data between client and central server
    thread client_to_central([client_socket, central_socket]() {
        forward_data(client_socket, central_socket);
    });

    client_to_log.join();
    client_to_central.join();

    closesocket(client_socket);
    closesocket(log_socket);
    closesocket(central_socket);
}

void forward_data(SOCKET source_socket, SOCKET destination_socket) {
    char buffer[BUFFER_SIZE];
    int bytes_read;
    while ((bytes_read = recv(source_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        send(destination_socket, buffer, bytes_read, 0);
    }
}


// MrDev25 - All Rights Reserved- 2024