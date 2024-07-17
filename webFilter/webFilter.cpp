// filtrado-web.cpp

#include "webFilter.h"
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <set>
#include <thread>

using namespace std;

set<string> blocked_sites = {"www.xvideos.com",
    "pornhub.com",
    "xvideos.com",
    "youporn.com",
    "redtube.com",
    "xnxx.com",
    "chaturbate.com",
    "livejasmin.com",
    "camsoda.com",
    "adfly.com",
    "adfoc.us",
    "linkbucks.com",

};

bool is_blocked(const string& host) {
    return blocked_sites.find(host) != blocked_sites.end();
}

string extract_host(const string& request) {
    size_t host_pos = request.find("Host: ");
    if (host_pos == string::npos) {
        return "";
    }
    size_t start = host_pos + 6; // Skip "Host: "
    size_t end = request.find("\r\n", start);
    return request.substr(start, end - start);
}



void handle_client(SOCKET client_socket) {
    char buffer[BUFFER_SIZE];
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_received <= 0) {
        closesocket(client_socket);
        return;
    }

    string request(buffer, bytes_received);
    string host = extract_host(request);

    if (is_blocked(host)) {
        string forbidden_response = "HTTP/1.1 403 Forbidden\r\nContent-Length: 0\r\n\r\n";
        send(client_socket, forbidden_response.c_str(), forbidden_response.length(), 0);
        closesocket(client_socket);
        return;
    }

    // Connect to the central server
    SOCKET central_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (central_socket == INVALID_SOCKET) {
        closesocket(client_socket);
        return;
    }

    sockaddr_in central_addr;
    central_addr.sin_family = AF_INET;
    central_addr.sin_port = htons(CENTRAL_SERVER_PORT);
    inet_pton(AF_INET, CENTRAL_SERVER_IP, &central_addr.sin_addr);

    if (connect(central_socket, (sockaddr*)&central_addr, sizeof(central_addr)) == SOCKET_ERROR) {
        closesocket(client_socket);
        closesocket(central_socket);
        return;
    }

    // Forward the request to the central server
    send(central_socket, request.c_str(), bytes_received, 0);

    // Forward response from central server back to client
    thread forward_to_client([&]() { forward(central_socket, client_socket); });
    forward_to_client.join();

    closesocket(client_socket);
    closesocket(central_socket);
}

void startFilter() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        cerr << "WSAStartup failed: " << result << "\n";
        return;
    }

    SOCKET filter_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (filter_socket == INVALID_SOCKET) {
        cerr << "Error creating filter socket: " << WSAGetLastError() << "\n";
        WSACleanup();
        return;
    }

    sockaddr_in filter_addr;
    filter_addr.sin_family = AF_INET;
    filter_addr.sin_port = htons(FILTER_PORT);
    filter_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(filter_socket, (sockaddr*)&filter_addr, sizeof(filter_addr)) == SOCKET_ERROR) {
        cerr << "Error binding filter socket: " << WSAGetLastError() << "\n";
        closesocket(filter_socket);
        WSACleanup();
        return;
    }

    if (listen(filter_socket, 10) == SOCKET_ERROR) {
        cerr << "Error listening on filter socket: " << WSAGetLastError() << "\n";
        closesocket(filter_socket);;
        WSACleanup();
        return;
    }

    cout << "Filter server listening on port " << FILTER_PORT << "\n";

    while (true) {
        SOCKET client_socket = accept(filter_socket, nullptr, nullptr);
        if (client_socket == INVALID_SOCKET) {
            cerr << "Error accepting connection: " << WSAGetLastError() << "\n";
            continue;
        }

        thread(handle_client, client_socket).detach();
    }

    closesocket(filter_socket);
    WSACleanup();
}

// MrDev25 - All Rights Reserved- 2024