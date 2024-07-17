#include "logs.h"

#define LOG_SERVER_PORT 9090
#define CENTRAL_SERVER_HOST "192.168.1.100" //IP of the centralized server
#define CENTRAL_SERVER_PORT 80
#define BUFFER_SIZE 4096

using namespace std;

void handle_logging(SOCKET client_socket);
void forward(SOCKET source_socket, SOCKET destination_socket);


void startLogger() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        cerr << "WSAStartup failed: " << result << "\n";
        return;
    }
    SOCKET log_server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (log_server_socket == INVALID_SOCKET) {
        cerr << "Error creating log server socket: " << WSAGetLastError() << "\n";
        return;
    }

    sockaddr_in log_server_addr;
    log_server_addr.sin_family = AF_INET;
    log_server_addr.sin_port = htons(LOG_SERVER_PORT);
    log_server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(log_server_socket, (sockaddr*)&log_server_addr, sizeof(log_server_addr)) == SOCKET_ERROR) {
        cerr << "Error binding log server socket: " << WSAGetLastError() << "\n";
        closesocket(log_server_socket);
        return;
    }

    if (listen(log_server_socket, 10) == SOCKET_ERROR) {
        cerr << "Error listening on log server socket: " << WSAGetLastError() << "\n";
        closesocket(log_server_socket);
        return;
    }

    cout << "Log server listening on port " << LOG_SERVER_PORT << "\n";

    while (true) {
        SOCKET client_socket = accept(log_server_socket, nullptr, nullptr);
        if (client_socket == INVALID_SOCKET) {
            cerr << "Error acceping connection: " << WSAGetLastError();
            continue;
        }
        thread(handle_logging, client_socket).detach();
    }

    closesocket(log_server_socket);
    WSACleanup();
}

void handle_logging(SOCKET client_socket) {
    SOCKET central_server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (central_server_socket == INVALID_SOCKET) {
        cerr << "Error creating internet socket: " << WSAGetLastError() << "\n";
        closesocket(client_socket);
        return;
    }
    // Address of the central server
    sockaddr_in central_server_addr;
    central_server_addr.sin_family = AF_INET;
    central_server_addr.sin_port = htons(CENTRAL_SERVER_PORT); // Central server port
    inet_pton(AF_INET, CENTRAL_SERVER_HOST, &central_server_addr.sin_addr); // Central server host

    if (connect(central_server_socket, (sockaddr*)&central_server_addr, sizeof(central_server_addr)) == SOCKET_ERROR) {
        cerr << "Error connecting to internet server: " << WSAGetLastError() << "\n";
        closesocket(client_socket);
        closesocket(central_server_socket);
        return;
    }

    // Threads for send data between clients and centralized server

    thread client_to_internet([&]() { forward(client_socket, central_server_socket); });
    thread internet_to_client([&]() { forward(central_server_socket, client_socket); });

    client_to_internet.join();
    internet_to_client.join();

    closesocket(client_socket);
    closesocket(central_server_socket);
}



// MrDev25 - All Rights Reserved- 2024

