#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include <winsock2.h>
#include <stdio.h>
#include "server.h"
#include "./../Shared/SocketSendRecvTools.h"

typedef struct _player Player;

int Winner();
void boardReset();
void TurnSwitchMessage(SOCKET players_socket);
int checkforwin();
void matrixToArray(char retPtr[10]);
void parseAndExecute(Player *player, char *data);


void BOARD_VIEW();
void USER_LIST_QUERY(Player *player, char *data, char *toSend);
void NEW_USER_REQUEST(Player *player, char *data, char *toSend);
void PLAY_REQUEST(Player *player, char *data, char *toSend);
void GAME_STATE_QUERY(Player *player, char *data, char *toSend);
void BOARD_VIEW_QUERY(Player *player, char *data, char *toSend);


int players_num;
char board[3][3];
bool game_started;

#endif
