// loadBalancer.cpp
#include "loadBalancer.h"
#include <iostream>
#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "curl\curl\include\curl\curl.h"

using namespace std;
#pragma comment(lib, "ws2_32.lib")

#define BALANCER_PORT 3128
#define CENTRAL_SERVER_PORT 9999
#define BUFFER_SIZE 4096

void forward(SOCKET source_socket, SOCKET destination_socket);

class RequestHandler {
public:
    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, string *data) {
        size_t realsize = size * nmemb;
        data->append((char*)contents, realsize);
        return realsize;
    }
};

void handle_client(SOCKET client_socket, const string& proxy_host, int proxy_port) {
    SOCKET proxy_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (proxy_socket == INVALID_SOCKET) {
        cerr << "Error creating the proxy socket: " << WSAGetLastError() << "\n";
        closesocket(client_socket);
        return;
    }

    sockaddr_in proxy_addr;
    proxy_addr.sin_family = AF_INET;
    proxy_addr.sin_port = htons(proxy_port);
    if (inet_pton(AF_INET, proxy_host.c_str(), &proxy_addr.sin_addr) <= 0) {
        cerr << "Invalid proxy address: " << proxy_host << "\n";
        closesocket(client_socket);
        closesocket(proxy_socket);
        return;
    }

    if (connect(proxy_socket, (sockaddr*)&proxy_addr, sizeof(proxy_addr)) == SOCKET_ERROR) {
        cerr << "Error connecting to proxy: " << WSAGetLastError() << "\n";
        closesocket(client_socket);
        closesocket(proxy_socket);
        return;
    }

    thread client_to_proxy([client_socket, proxy_socket]() { forward(client_socket, proxy_socket); });
    thread proxy_to_client([proxy_socket, client_socket]() { forward(proxy_socket, client_socket); });

    client_to_proxy.join();
    proxy_to_client.join();

    closesocket(client_socket);
    closesocket(proxy_socket);
}


void startBalancer() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        cerr << "WSAStartup failed: " << result << "\n";
        return;
    }

    SOCKET balancer_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (balancer_socket == INVALID_SOCKET) {
        cerr << "Error creating the balancer socket: " << WSAGetLastError() << "\n";
        WSACleanup();
        return;
    }

    sockaddr_in balancer_addr;
    balancer_addr.sin_family = AF_INET;
    balancer_addr.sin_port = htons(BALANCER_PORT);
    balancer_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(balancer_socket, (sockaddr*)&balancer_addr, sizeof(balancer_addr)) == SOCKET_ERROR) {
        cerr << "Error binding the balancer socket: " << WSAGetLastError() << "\n";
        closesocket(balancer_socket);
        WSACleanup();
        return;
    }

    if (listen(balancer_socket, 10) == SOCKET_ERROR) {
        cerr << "Error listening on the balancer socket: " << WSAGetLastError() << "\n";
        closesocket(balancer_socket);
        WSACleanup();
        return;
    }

    cout << "Load balancer listening on port " << BALANCER_PORT << "\n";

    while (true) {
        SOCKET client_socket = accept(balancer_socket, nullptr, nullptr);
        if (client_socket == INVALID_SOCKET) {
            cerr << "Error accepting connection: " << WSAGetLastError() << "\n";
            continue;
        }

        // HTTP request to central server to get proxy address
        CURL *curl;
        CURLcode res;
        string proxy_host;
        int proxy_port;

        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();
        if (curl) {
            string url = "http://127.0.0.1:9999/get_proxy";
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            string response;
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, RequestHandler::WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

            res = curl_easy_perform(curl);

            if (res != CURLE_OK) {
                cerr << "Error getting proxy address: " << curl_easy_strerror(res) << endl;
                curl_easy_cleanup(curl);
                closesocket(client_socket);
                continue;
            }

            curl_easy_cleanup(curl);

            // Assuming the format of the central server response is "proxy_host:proxy_port"
            size_t colon_pos = response.find(":");
            if (colon_pos != string::npos) {
                proxy_host = response.substr(0, colon_pos);
                proxy_port = stoi(response.substr(colon_pos + 1));
            } else {
                cerr << "Invalid central server response: " << response << endl;
                closesocket(client_socket);
                continue;
            }
        }

        // Start a new thread to handle client request with obtained proxy address
        thread(handle_client, client_socket, proxy_host, proxy_port).detach();
    }

    closesocket(balancer_socket);
    WSACleanup();
}

void forward(SOCKET source_socket, SOCKET destination_socket) {
    char buffer[BUFFER_SIZE];
    int bytes_read;
    while ((bytes_read = recv(source_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        send(destination_socket, buffer, bytes_read, 0);
    }
}
// MrDev25 - All Rights Reserved- 2024