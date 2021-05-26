#include "server.h"

void main(int argc, char *argv[])
{
	int exitCode = ServerSetup(argv); // server setup
	if (exitCode == 0)
		exit(0);
	else if (exitCode == 1)
		goto server_cleanup_1;
	else if (exitCode == 2)
		goto server_cleanup_2;

	while (!finish)
	{// This loop is for restarting the server once a game has been finished or once a user disconnects.
		players_num = 0;
		boardReset();
		ThreadHandles[2] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ConnectionsThread, NULL, 0, NULL);
		ThreadHandles[3] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)KillThread, NULL, 0, NULL);
		if (ThreadHandles[2] == NULL || ThreadHandles[3] == NULL)
		{
			fprintf(log, "Thread creation failed. Exiting.\n");
			fclose(log);
			exit(0);
		}
		if(WaitForSingleObject(restartSem, INFINITE)==WAIT_FAILED)
		{
			fprintf(log, "WaitForMultipleObjects failed. Exiting.\n");
			fclose(log);
			exit(0);
		}
		restart = true;
		WaitForMultipleObjects(4, ThreadHandles, TRUE, INFINITE);
		for (int i = 0; i < NUM_OF_THREADS; i++)
		{
			if (ThreadHandles[i] != NULL)
				if (!CloseHandle(ThreadHandles[i]))
				{
					fprintf(log, "CloseHandle failed, exiting.\n");
					fclose(log);
					exit(1);
				}
			ThreadHandles[i] = NULL;
		}
		fprintf(log,"Player disconnected. Exiting.\n");
		printf("Player disconnected. Exiting.\n");
		for (int i = 0; i < 2;i++)
		if (players[i].online)
		{
			if (closesocket(players[i].socket) != 0)
			{
				fprintf(log, "CloseHandle failed, exiting.\n");
				fclose(log);
				exit(0);
			}
		}
		playersReset();
		mutexAndSemaphoresReset();
		resetThreadHandles();
		restart = false;
	}
	printf("No players connected. Exiting.\n");
	fprintf(log, "No players connected. Exiting.\n");
	fclose(log);

server_cleanup_2:
	if (closesocket(MainSocket) == SOCKET_ERROR)
		printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());

server_cleanup_1:
	if (WSACleanup() == SOCKET_ERROR)
		printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError());
}

void playersReset()
{//Resets the players array
	for (int i = 0; i < 2; i++)
	{
		players[i].name[0] = '\0';
		players[i].symbol = 'z';
		players[i].first_message_sent = false;
		players[i].online = false;
		players[i].turn = false;
	}
}



int ServerSetup(char *argv[])
{//This function sets up the serve, resets the necessary variables and sets up the TCP protocol.
	log_path = argv[1];
	log = fopen(log_path, "w");
	if (log == NULL)
		exit(0);
	game_started = false;
	MainSocket = INVALID_SOCKET;
	SOCKADDR_IN service;
	int bindRes;
	int ListenRes;
	WSADATA wsaData;

	restartSem = NULL;
	requestMutex = NULL;
	mutex1 = NULL;
	mutex2 = NULL;
	game_start_sem = NULL;
	killSem = NULL;
	restart = false;
	finish = false;
	playersReset();
	mutexAndSemaphoresReset();
	resetThreadHandles();

	int StartupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (StartupRes != NO_ERROR)
		return 0;

	MainSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (MainSocket == INVALID_SOCKET)
	{
		printf("Error at socket( ): %ld\n", WSAGetLastError());
		return 1;
	}
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = INADDR_ANY;
	service.sin_port = htons((unsigned short)strtoul(argv[2], NULL, 0));
	bindRes = bind(MainSocket, (SOCKADDR*)&service, sizeof(service));
	if (bindRes == SOCKET_ERROR)
	{
		printf("bind( ) failed with error %ld. Ending program\n", WSAGetLastError());
		return 2;
	}
	ListenRes = listen(MainSocket, SOMAXCONN);
	if (ListenRes == SOCKET_ERROR)
	{
		printf("Failed listening on socket, error %ld.\n", WSAGetLastError());
		return 2;
	}
	for (int i = 0; i < NUM_OF_THREADS; i++)
		ThreadHandles[i] = NULL;
	return -1;
}



static DWORD ServiceThread(Player *player)
{// this is the service thread, it is responsible for communication with a client once he's been accepted to the game
	player->online = true;
	if (restart)
		return 1;
	WaitForSingleObject(mutex1, INFINITE);				//some setup necessary for each of the clients											
	//critical section starts
	if (players[0].name[0] == '\0' && players[1].name[0] == '\0')
	{
		player->symbol = 'x';
		players_num = 1;
	}
	else
	{
		player->symbol = 'o';
		players_num = 2;
	}
	if(player->name[0]=='\0')
		player->name[0] = ' ';
	if (restart)
		return 1;
	ReleaseMutex(mutex1);
	//critical section ends
	if (restart)
		return 1;
	ReleaseSemaphore(killSem, 1, NULL);
	char *SendStr = (char*)malloc(sizeof(char)*SEND_STR_SIZE);
	if (SendStr == NULL)
	{
		if (restart)
			return 1;
		WaitForSingleObject(log_mutex, INFINITE);
		fprintf(log, "Memory allocation failed, Exiting.\n");
		exit(0);
	}
	TransferResult_t RecvRes;
	while (!restart)
	{// The communication loop, in each iteration a message is sent and received from the client.
		char *AcceptedStr = NULL;
		if (restart)
			return 1;
		WaitForSingleObject(mutex2, INFINITE);
		if (players[0].online && players[1].online && player->first_message_sent == false && players[0].name[0]!=' ' && players[1].name[0] != ' ')
			GameStart(SendStr);
		if (restart)
			return 1;
		ReleaseMutex(mutex2);
		RecvRes = ReceiveString(&AcceptedStr, player->socket);
		if (RecvRes == TRNS_FAILED)
		{
			
			AcceptedStr = (char *)malloc(sizeof(char));
			AcceptedStr[0] = '\0';
			restart = true;
			ReleaseSemaphore(restartSem, 1, NULL);
			if (SendStr != NULL)
				free(SendStr);
		}
		if (restart)
			return 1;
		WaitForSingleObject(requestMutex, INFINITE);
		parseAndExecute(player, AcceptedStr);
		if (restart)
			return 1;
		ReleaseMutex(requestMutex);
		free(AcceptedStr);
	}
	if (SendStr != NULL)
		free(SendStr);
	return 0;
}

void GameStart(char *SendStr)
{// This function starts the game and sends the the messages to the clients when it starts.
	players[0].first_message_sent = true;
	players[1].first_message_sent = true;
	snprintf(SendStr, SEND_STR_SIZE, "GAME_STARTED\n\0");
	SendAndCheck(SendStr, players[0].socket);
	SendAndCheck(SendStr, players[1].socket);
	BOARD_VIEW();
	if (players[0].symbol == 'x')
		players[0].turn = true;
	else
		players[1].turn = true;
	TurnSwitchMessage(players[0].socket);
	TurnSwitchMessage(players[1].socket);
	game_started = true;
}

static DWORD ConnectionsThread(void)
{// This thread is responsible for accepting new connections to the server
	char toSend[SEND_STR_SIZE];
	while (TRUE)
	{	
		isNameTaken = false;
		if (restart) 
			return 0;
		int i;
		SOCKET AcceptSocket = accept(MainSocket, NULL, NULL);

		if (restart || finish) return 0;
		if (AcceptSocket == INVALID_SOCKET)
		{
			printf("Accepting connection with client failed, error %ld\n", WSAGetLastError());
			return 1;
		}
		if (players_num==2)
		{
			snprintf(toSend, SEND_STR_SIZE, "NEW_USER_DECLINED\n\0");
			SendAndCheck(toSend, AcceptSocket);
			closesocket(AcceptSocket);
		}

		else
		{
			if (players_num == 0)
			{
				resetThreadHandles();
				playersReset();
				players[0].socket = AcceptSocket;
				i = 0;
			}
			else
			{
				if (ThreadHandles[0] == NULL) i = 0;
				else i = 1;
				players[i].socket = AcceptSocket;
				char *AcceptedStr = (char *)malloc(sizeof(char)*SEND_STR_SIZE);
				TransferResult_t RecvRes = ReceiveString(&AcceptedStr, AcceptSocket);
				if (RecvRes == TRNS_FAILED)
				{
					AcceptedStr = (char *)malloc(sizeof(char));
					AcceptedStr[0] = '\0';
					if (!restart)
						ReleaseSemaphore(restartSem, 1, NULL);
				}
				parseAndExecute(&players[i], AcceptedStr);
				if (isNameTaken)
				{
					if(closesocket(players[i].socket)!=0)
					{
						fprintf(log, "CloseHandle failed, exiting.\n");
						fclose(log);
						exit(0);
					}
				}
			}
			if (!isNameTaken)
			{
				ThreadHandles[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ServiceThread, &players[i], 0, NULL);
				if (ThreadHandles[i] == NULL)
				{
					fprintf(log, "Thread creation failed. Exiting.\n");
					fclose(log);
					exit(0);
				}
			}
		}
	}
	return 0;
}

int SendAndCheck(char *SendStr, SOCKET playersSocket)
{// This thread sends a string to a players socket, if it fails it restarts the server.
	TransferResult_t SendRes;
	if (SendStr[0] != '\0')
	{
		SendRes = SendString(SendStr, playersSocket);
		if (SendRes == TRNS_FAILED)
		{
			if (!restart)
				ReleaseSemaphore(restartSem, 1, NULL);
		}
	}
	return 0;
}

static DWORD KillThread(void)
{//This thread determines whether a user connected within 5 minutes and if not it causes the program to end
	while (players_num != 2)
	{
		if (restart) return 0;
		if(WaitForSingleObject(killSem, 300000)==WAIT_FAILED)
		{
			fprintf(log, "WaitForSingleObject failed, exiting.\n");
			exit(0);
		}
		if (restart) return 0;
		if (players_num == 0)
		{
			if(TerminateThread(ThreadHandles[2], 1)==0)
			{
				fprintf(log, "Thread termination failed, exiting.\n");
				fclose(log);
				exit(0);
			}
			finish = true;
			if(ReleaseSemaphore(restartSem, 1, NULL)==0)
			{
				fprintf(log, "ReleaseSemaphore failed, exiting.\n");
				exit(0);
			}
			return 0;
		}
	}
	return 0;
}


