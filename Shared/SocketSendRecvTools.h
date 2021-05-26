#ifndef SOCKET_SEND_RECV_TOOLS_H
#define SOCKET_SEND_RECV_TOOLS_H

#define SERVER_ADDRESS_STR "127.0.0.1"
#define SERVER_PORT 2345

#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

typedef enum { TRNS_FAILED, TRNS_DISCONNECTED, TRNS_SUCCEEDED } TransferResult_t;

TransferResult_t SendString(const char *Str, SOCKET sd);
TransferResult_t ReceiveString(char** OutputStrPtr, SOCKET sd);


#endif 