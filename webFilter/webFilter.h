#ifndef WEB_FILTER_H
#define WEB_FILTER_H

#include <string>
#include <set>
#include <winsock2.h>

using namespace std;

//Definition of constants
#define FILTER_PORT 8888
#define BUFFER_SIZE 4096
#define CENTRAL_SERVER_PORT 9999
#define CENTRAL_SERVER_IP "127.0.0.1"

// Declaration of global changeable
extern set<string> blocked_sites;

// Declaration of functions
bool is_blocked(const string& host);
string extract_host(const string& request);
void forward(SOCKET source_socket, SOCKET destination_socket);
void handle_client(SOCKET client_socket);
void startFilter();

#endif // WEB_FILTER_H

// MrDev25 - All Rights Reserved- 2024