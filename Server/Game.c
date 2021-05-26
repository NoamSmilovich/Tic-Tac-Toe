#include "Game.h"


int Winner() //return an integer based on the boards current state.
{// 0- its a tie ; 1- x won ; 2- o won ; -1 board isn't full, no winner yet
	int check = checkforwin();
	if (check == -1) return 2;
	else if (check == 1) return 1;
	else
	{
		for (int i = 0; i < 3; i++)
			for (int j = 0; j < 3; j++)
				if (board[i][j] == ' ')
					return -1;
		return 0;
	}
}


void USER_LIST_QUERY(Player *player, char *data, char *toSend)
{//Sends the client a user list reply
	if (players[0].symbol == 'x')
		snprintf(toSend, SEND_STR_SIZE, "USER_LIST_REPLY:x;%s", players[0].name);
	else if (players[1].symbol == 'x')
		snprintf(toSend, SEND_STR_SIZE, "USER_LIST_REPLY:x;%s", players[0].name);

	if (players[0].symbol == 'o' || players[1].symbol == 'o') // 2 players connected
		strcat(toSend, ";o;");
	if (players[0].symbol == 'o')
		strcat(toSend, players[0].name);
	else
		strcat(toSend, players[1].name);
	strcat(toSend, "\n\0");
	SendAndCheck(toSend, player->socket);
}

void NEW_USER_REQUEST(Player *player, char *data, char *toSend)
{//Sends the client a message if he's been accepted or declined from the server
	char name[MAX_NAME_LENGTH];
	strcpy(name, data + 17);
	name[strlen(name) - 1] = '\0';
	if (strcmp(players[0].name, name) == 0 || strcmp(players[1].name, name) == 0)
	{
		isNameTaken = true;
		snprintf(toSend, SEND_STR_SIZE, "NEW_USER_DECLINED\n\0");
	}
	else
	{
		strcpy(player->name, name);
		if(player->symbol=='x')
			snprintf(toSend, SEND_STR_SIZE, "NEW_USER_ACCEPTED:%d;x\n\0", players_num);
		else
			snprintf(toSend, SEND_STR_SIZE, "NEW_USER_ACCEPTED:%d;o\n\0", players_num);
	}
	SendAndCheck(toSend, player->socket);
}

void PLAY_REQUEST(Player *player, char *data, char *toSend)
{//Responds to a play request, if the move is accepted the board changes and both clients are notified
	//if as a result the game ends, both clients are notified.
	//if the move isnt accepted the function sends the client a play declined message and the reason for it.
	bool end_game = false;
	int winner = -2;
	int x = data[13] - '0' - 1, y = data[15] - '0' - 1;
	if (!game_started)
		strcpy(toSend, "PLAY_DECLINED:Game has not started\n\0");
	else
	{
		if (!player->turn)
			strcpy(toSend, "PLAY_DECLINED:Not your turn\n\0");
		else
		{
			if (x > 2 || x < 0 || y>2 || y < 0 || board[x][y] != ' ')
				strcpy(toSend, "PLAY_DECLINED:Illegal move\n\0");
			else
			{

				board[x][y] = player->symbol;
				strcpy(toSend, "PLAY_ACCEPTED\n\0");
				players[0].turn = !players[0].turn;
				players[1].turn = !players[1].turn;
				winner = Winner();
				if (winner != -1)
				{
					restart = true;
					SendAndCheck(toSend, player->socket);
					if (winner == 0)
					{
						BOARD_VIEW();
						strcpy(toSend, "GAME_ENDED:TIE\n\0");
					}
					else
					{
						char winMsg[SEND_STR_SIZE];
						winMsg[0] = '\0';
						if (winner == 1 && players[0].symbol == 'x')
							snprintf(winMsg, SEND_STR_SIZE, "GAME_ENDED:WIN_%s\n\0", players[0].name);
						else if (winner == 1 && players[1].symbol == 'x')
							snprintf(winMsg, SEND_STR_SIZE, "GAME_ENDED:WIN_%s\n\0", players[1].name);
						else if (winner == 2 && players[0].symbol == 'o')
							snprintf(winMsg, SEND_STR_SIZE, "GAME_ENDED:WIN_%s\n\0", players[0].name);
						else if (winner == 2 && players[1].symbol == 'o')
							snprintf(winMsg, SEND_STR_SIZE, "GAME_ENDED:WIN_%s\n\0", players[1].name);
						BOARD_VIEW();
						SendAndCheck(winMsg, players[0].socket);
						SendAndCheck(winMsg, players[1].socket);
						toSend[0] = '\0';
					}
				}
			}
		}
	}
	SendAndCheck(toSend, player->socket);
	if (winner == -1)
	{
		BOARD_VIEW();
		TurnSwitchMessage(players[0].socket);
		TurnSwitchMessage(players[1].socket);
	}
}

void GAME_STATE_QUERY(Player *player, char *data, char *toSend)
{//Sends the user a game state query
	if (!game_started)
		snprintf(toSend, SEND_STR_SIZE, "GAME_STATE_REPLY:0\n\0");
	else
	{
		if (players[0].turn)
			snprintf(toSend, SEND_STR_SIZE, "GAME_STATE_REPLY:%c;%s\n\0", players[0].symbol, players[0].name);
		else
			snprintf(toSend, SEND_STR_SIZE, "GAME_STATE_REPLY:%c;%s\n\0", players[1].symbol, players[1].name);
	}
	SendAndCheck(toSend, player->socket);

}

void BOARD_VIEW_QUERY(Player *player, char *data, char *toSend)
{//Sends the user a board view query
	char boardInString[10];
	matrixToArray(boardInString);
	snprintf(toSend, SEND_STR_SIZE, "BOARD_VIEW_REPLY:%s\n\0", boardInString);
	SendAndCheck(toSend, player->socket);

}

void parseAndExecute(Player *player, char *data)
{//Decrypts the message type it receives and calls the necessary function
	char *toSend = (char *)malloc(sizeof(char)*SEND_STR_SIZE);
	if (toSend == NULL)
		exit(0);
	toSend[0] = '\0';
	if (strncmp(data, "USER_LIST_QUERY", 15) == 0)
		USER_LIST_QUERY(player, data, toSend);
	if (strncmp(data, "NEW_USER_REQUEST:", 17) == 0)
		NEW_USER_REQUEST(player, data, toSend);
	if (strncmp(data, "PLAY_REQUEST:", 13) == 0)
		PLAY_REQUEST(player, data, toSend);
	if (strncmp(data, "GAME_STATE_QUERY", 16) == 0)
		GAME_STATE_QUERY(player, data, toSend);
	if (strncmp(data, "BOARD_VIEW_QUERY", 16) == 0)
		BOARD_VIEW_QUERY(player, data, toSend);
	free(toSend);
}

void boardReset()
{// resets the board
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			board[i][j] = ' ';
}

void matrixToArray(char retPtr[10])
{//turns the board to a string, so we can sends it to a client
	char a[] = { board[0][0] ,board[0][1] ,board[0][2] ,board[1][0] ,board[1][1] ,board[1][2] ,board[2][0] ,board[2][1] ,board[2][2] ,'\0' };
	strcpy(retPtr, a);
	retPtr[9] = '\0';
}

void TurnSwitchMessage(SOCKET players_socket)
{// sends both clients a turn switch message
	char *SendStr = (char *)malloc(sizeof(char)*SEND_STR_SIZE);
	if (players[0].first_message_sent)
	{
		if (players[0].turn)
			snprintf(SendStr, SEND_STR_SIZE, "TURN_SWITCH:%s;%c\n\0", players[0].name, players[0].symbol);
		else
			snprintf(SendStr, SEND_STR_SIZE, "TURN_SWITCH:%s;%c\n\0", players[1].name, players[1].symbol);
		SendAndCheck(SendStr, players_socket);
	}
	free(SendStr);
}

int checkforwin()
{// a function that helps Winner(), checks the board for a winner
	for (int x = 0; x < 3; x++)
	{
		// check vertical lines
		if ((board[x][0] != ' ') &&
			(board[x][0] == board[x][1]) &&
			(board[x][0] == board[x][2]))
			return(board[x][0] == 'o' ? -1 : 1);

		// check horizontal lines
		if ((board[0][x] != ' ') &&
			(board[0][x] == board[1][x]) &&
			(board[0][x] == board[2][x]))
			return(board[0][x] == 'o' ? -1 : 1);
	};

	// check top left to bottom right diagonal line
	if ((board[0][0] != ' ') &&
		(board[0][0] == board[1][1]) &&
		(board[0][0] == board[2][2]))
		return(board[0][0] == 'o' ? -1 : 1);

	// check bottom left to top right diagonal line
	if ((board[2][0] != ' ') &&
		(board[2][0] == board[1][1]) &&
		(board[2][0] == board[0][2]))
		return(board[2][0] == 'o' ? -1 : 1);

	// no winner
	return 0;
}

void BOARD_VIEW()
{// Sends a board view message to both clients
	char *SendStr = (char *)malloc(sizeof(char)*SEND_STR_SIZE);
	char matInStr[10];
	matrixToArray(matInStr);
	snprintf(SendStr, SEND_STR_SIZE, "BOARD_VIEW:%s\n\0", matInStr);
	SendAndCheck(SendStr, players[0].socket);
	SendAndCheck(SendStr, players[1].socket);
	free(SendStr);
}