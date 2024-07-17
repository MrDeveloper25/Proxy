#include <iostream>
#include <thread>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <fstream>
#include <conio.h>
#include <mutex>
#include <windows.h>


#pragma comment(lib, "ws2_32.lib")

// Constants for ports and buffer size
#define BALANCER_PORT 3128
#define FILTER_PORT 8888
#define LOG_SERVER_PORT 9090
#define PROXY_PORT 8192
#define LOCAL_PROXY_PORT 8882
#define CENTRAL_SERVER_PORT 9999
#define BUFFER_SIZE 4096

using namespace std;

// Mutex for logging to avoid race conditions
mutex logMutex;

// Prototypes of functions
void startBalancer();
void startFilter();
void startLogger();
void startProxy();
void startProxyWeb();
void connectToCentralServer(const char* moduleName, int modulePort);
void handleCentralServer();
void handleClient(SOCKET clientSocket, int serverPort);
void handleLogging(SOCKET clientSocket);


bool connectToServerWithRetries(SOCKET& socket, const char* ip, int port, int maxAttempts) {
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &serverAddr.sin_addr);

    for (int attempt = 1; attempt <= maxAttempts; ++attempt) {
        cout << "Attempting to connect to " << ip << " on port " << port << "(Attempt " << attempt << ")" << "\n";
        if (connect(socket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            cerr << "Failed to connect to " << ip << " on port " << port << ", error: " << WSAGetLastError() << "\n";
            if (attempt < maxAttempts) {
                // Retry after a short delay
                this_thread::sleep_for(chrono::seconds(2));
                continue;
            } else {
                return false;
            }
        }
        return true;
    }
    return false;
}


int main() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        cerr << "WSAStartup failed: " << result << "\n";
        return 1;
    }
    // Start the central server and wait for it to be ready
    thread centralServerThread(handleCentralServer);
    this_thread::sleep_for(chrono::seconds(2)); // Wait for the central server to start


    // Connect and register each module with the centralized server
    connectToCentralServer("Balancer", BALANCER_PORT);
    connectToCentralServer("Filter", FILTER_PORT);
    connectToCentralServer("Logger", LOG_SERVER_PORT);
    connectToCentralServer("Proxy", PROXY_PORT);
    connectToCentralServer("ProxyWeb", LOCAL_PROXY_PORT);

    // Create threads for each server
    thread balancerThread(startBalancer);
    thread filterThread(startFilter);
    thread loggerThread(startLogger);
    thread proxyThread(startProxy);
    thread proxyWebThread(startProxyWeb);



   
    // Wait the Threads end
    balancerThread.join();
    filterThread.join();
    loggerThread.join();
    proxyThread.join();
    proxyWebThread.join();

    // Wait a thread for each local centralized server end
    centralServerThread.join();

    WSACleanup();
    return 0;

}

// Function to connect and register a module with the central server
void connectToCentralServer(const char *moduleName, int modulePort) {
    SOCKET moduleSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (moduleSocket == INVALID_SOCKET) {
        cerr << "Error creating module socket: " << WSAGetLastError() << "\n";
        return;
    }

    // Attempt to connect to central server with retries
    if (!connectToServerWithRetries(moduleSocket, "127.0.0.1", CENTRAL_SERVER_PORT, 3)) {
        cerr << "Failed to connect to central server for module registration." << endl;
        closesocket(moduleSocket);
        return;
    }

    // Send module name and port to central server for registration
    string registrationMessage = string(moduleName) + " " + to_string(modulePort);
    send(moduleSocket, registrationMessage.c_str(), registrationMessage.length(), 0);

    closesocket(moduleSocket);
}

void handleCentralServer() {
    SOCKET centralServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (centralServerSocket == INVALID_SOCKET) {
        cerr << "Error creating central server socket: " << WSAGetLastError() << "\n";
        return;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(CENTRAL_SERVER_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(centralServerSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Error binding central server socket: " << WSAGetLastError() << "\n";
        closesocket(centralServerSocket);
        return;
    }

    if (listen(centralServerSocket, 10) == SOCKET_ERROR) {
        cerr << "Error listening on central server socket: " << WSAGetLastError() << "\n";
        closesocket(centralServerSocket);
        return;
    }

    cout << "Central server listening on port " << CENTRAL_SERVER_PORT << "\n";

    while (true) {
        SOCKET clientSocket = accept(centralServerSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            cerr << "Error accepting connection: " << WSAGetLastError() << "\n";
            continue;
        }
        thread(handleClient, clientSocket, CENTRAL_SERVER_PORT).detach();
    }
    closesocket(centralServerSocket);
}

void handleClient(SOCKET clientSocket, int serverPort) {
    char buffer[BUFFER_SIZE];
    int bytesReceived;

    // Receive module registration message
    bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';
        cout << "Received registration: " << buffer << "\n";
    } else {
        cerr << "Error receiving registration message\n";
        closesocket(clientSocket);
        return;
    }

    // Echo back to acknowledge registration
    send(clientSocket, buffer, bytesReceived, 0);

    closesocket(clientSocket);
}


void handleLogging(SOCKET clientSocket) {
    char buffer[BUFFER_SIZE];
    int bytesReceived;

    while ((bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[bytesReceived] = '\0';

        // Save the register in a file
        lock_guard<mutex> guard(logMutex);
        ofstream logFile("logs.txt", ios::app);
        if (logFile.is_open()) {
            logFile << buffer << "\n";
            logFile.close();
        }

        // Send the data at the process of visualization in real time
    
        cout << buffer << "\n";
    }

    if (bytesReceived == SOCKET_ERROR) {
        cerr << "Error receiving data: " << WSAGetLastError() << "\n";
    }

    closesocket(clientSocket);
}



void startProxyWeb() {
    SOCKET proxyWebSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (proxyWebSocket == INVALID_SOCKET) {
        cerr << "Error creating the web proxy socket: " << WSAGetLastError() << "\n";
        return;
    }

    sockaddr_in proxyWebAddr;
    proxyWebAddr.sin_family = AF_INET;
    proxyWebAddr.sin_port = htons(LOCAL_PROXY_PORT);
    proxyWebAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(proxyWebSocket, (sockaddr*)&proxyWebAddr, sizeof(proxyWebAddr)) == SOCKET_ERROR) {
        cerr << "Error binding the web proxy socket: " << WSAGetLastError() << "\n";
        closesocket(proxyWebSocket);
        return;
    }

    if (listen(proxyWebSocket, 10) == SOCKET_ERROR) {
        cerr << "Error listening on the web proxy socket: " << WSAGetLastError() << "\n";
        closesocket(proxyWebSocket);
        return;
    }

    cout << "Web proxy listening on port " << LOCAL_PROXY_PORT << "\n";

    while (true) {
        SOCKET clientSocket = accept(proxyWebSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            cerr << "Error accepting connection: " << WSAGetLastError() << "\n";
            continue;
        }

        thread(handleClient, clientSocket, LOCAL_PROXY_PORT).detach();
    }

    closesocket(proxyWebSocket);
}
