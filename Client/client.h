#ifndef CLIENT_H
#define CLIENT_H

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <stdbool.h>
#include <stdlib.h>
#include ".\..\Shared\SocketSendRecvTools.h"
#include "MsgParse.h"

#define SEND_STR_SIZE 100
#define MAX_NAME_LENGTH 30

static DWORD Interface(void);
static DWORD RecvDataThread(void);
static DWORD SendDataThread(char *name);
void ClientSetup(char *argv[]);

char *log_path;
char *ip;
char *port_str;
char toSend[SEND_STR_SIZE];
SOCKET m_socket;
u_short port;
HANDLE SendingSem;
HANDLE InputSem;
HANDLE hThread[3];
HANDLE log_mutex;
bool end;
FILE *log;


#endif 