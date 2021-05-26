#ifndef SERVER_H
#define SERVER_H

#include "Game.h"
#include <stdio.h>
#include <string.h>
#include <winsock2.h>

#include <stdbool.h>
#include ".\..\Shared\SocketSendRecvTools.h"
#include "Threads.h"
#include <stdlib.h>

#define SEND_STR_SIZE 100
#define MAX_NAME_LENGTH 30

typedef struct _player {
	char name[MAX_NAME_LENGTH];
	char symbol;
	SOCKET socket;
	bool turn;
	bool first_message_sent;
	bool online;
}Player;

char *log_path;
Player players[2];
SOCKET MainSocket;
bool restart;
bool finish;
bool isNameTaken;
FILE *log;

static DWORD ServiceThread(Player *player);
int ServerSetup(char *argv[]);
void parseAndExecute(Player *player, char *data);
static DWORD ConnectionsThread(void);
static DWORD KillThread(void);
int SendAndCheck(char *SendStr, SOCKET playersSocket);
void playersReset();
void GameStart(char *SendStr);


#endif