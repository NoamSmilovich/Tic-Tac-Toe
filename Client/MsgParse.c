#include "MsgParse.h"

void parseData(char *data)
{//Decrypts the message received from the server and prints a message according to it
	if (strncmp(data, "NEW_USER_DECLINED",17) == 0)
	{
		printf("Request to join was refused.\n");
		if (TerminateThread(hThread[2], 0) == 0)
		{
			fprintf(log, "Thread termination failed, exiting.\n");
			exit(0);
		}
		shutdown(m_socket, 2);
		end = true;
	}
	else if (strncmp(data, "BOARD_VIEW", 10) == 0)
		BoardView(data);
	else if (strncmp(data, "NEW_USER_ACCEPTED",17) == 0)
		printf("Connected to server on %s:%s\n", ip, port_str);
	else if (strncmp(data, "GAME_STARTED",12) == 0)
		printf("Game is on!\n");
	else if (strncmp(data, "TURN_SWITCH:", 12) == 0)
	{
		char name[MAX_NAME_LENGTH];
		int i = 0;
		while (data[i + strlen("TURN_SWITCH:")] != ';')
		{
			name[i] = data[i + strlen("TURN_SWITCH:")];
			i++;
		}
		name[i] = '\0';
		printf("%s's turn (%c)\n", name, data[strlen(data)-2]);
	}
	else if (strncmp(data, "GAME_ENDED", 10) == 0)
	{
		if (strncmp(data + strlen("GAME_ENDED:"), "TIE", 3)==0)
			printf("Game ended. Everybody wins!\n");
		else
		{
			char name[MAX_NAME_LENGTH];
			strcpy(name, data + strlen("GAME_ENDED:WIN_"));
			name[strlen(name) - 1] = '\0';
			printf("Game ended. The winner is %s!\n", name);
		}
		end = true;
	}
	else if (strncmp(data, "PLAY_DECLINED", 13) == 0)
		printf("Error: %s",data+14);
	else if (strncmp(data, "USER_LIST_REPLY:", 16) == 0)
	{		
		char out[SEND_STR_SIZE];
		strcpy(out, data + strlen("USER_LIST_REPLY:"));
		for (int j = 0; j < strlen(out); j++)
			if (out[j] == ';')
				out[j] = ':';
		int i = 0, counter=0;
		while (i < strlen(out))
		{
			if (out[i] == ':')
				counter++;
			i++;
			if (counter == 2)
				break;
		}
		if(i!=strlen(out))
		{
			char temp[SEND_STR_SIZE];
			snprintf(temp, SEND_STR_SIZE, ", %s", out + i);
			strcpy(out + i, temp);
		}
		printf("Players: %s", out);
	}
	else if (strncmp(data, "GAME_STATE_REPLY:", 17) == 0)
	{
		if (data[17] == '0')
			printf("Game has not started\n");
		else
		{
			char name[MAX_NAME_LENGTH];
			strcpy(name, data + strlen("GAME_STATE_REPLY:z;"));
			name[strlen(name) - 1] = '\0';
			printf("%s's turn (%c)\n", name, data[strlen("GAME_STATE_REPLY:")]);
		}
	}
	else if (strncmp(data, "PLAY_ACCEPTED", 13) == 0)
	{
		printf("Well played\n");
	}
	else
	{
		if(WaitForSingleObject(log_mutex, INFINITE)==WAIT_FAILED)
		{
			fprintf(log, "WaitForSingleObject failed, exiting.\n");
			exit(0);
		}
		printf("Received weird message from server, exiting\n");
		fprintf(log, "Received weird message from server, exiting\n");
		fclose(log);
		if(ReleaseMutex(log_mutex)==0)
		{
			fprintf(log, "ReleaseMutex failed, exiting.\n");
			exit(0);
		}
		exit(1);
	}
}

void parseUserInput(char *input, char *SendStr)
{//parses the input from the user and sends a request to the client according to it
	//if input is wrong, it prints an illegal command message
	if (strcmp(input, "players") == 0)
		snprintf(SendStr, SEND_STR_SIZE, "USER_LIST_QUERY\n\0");
	else if (strncmp(input, "play", 4) == 0)
		snprintf(SendStr, SEND_STR_SIZE, "PLAY_REQUEST:%c;%c\n\0", input[5], input[7]);
	else if (strncmp(input, "state", 5) == 0)
		snprintf(SendStr, SEND_STR_SIZE, "GAME_STATE_QUERY\n\0");
	else if (strncmp(input, "board", 5) == 0)
		snprintf(SendStr, SEND_STR_SIZE, "BOARD_VIEW_QUERY\n\0");
	else if ((strncmp(input, "exit", 4) == 0))
		end = true;
	else
	{
		printf("Illegal command\n");
		SendStr[0] = '\0';
	}
}

void BoardView(char* data)
{//prints the current state of the board according to the data received from the server
	if(strncmp(data,"BOARD_VIEW_REPLY:",17)==0)
		data = data + strlen("BOARD_VIEW_REPLY:");
	else
		data = data + strlen("BOARD_VIEW:");
	printf("|%c|%c|%c|\n", data[6], data[7], data[8]);
	printf("|%c|%c|%c|\n", data[3], data[4], data[5]);
	printf("|%c|%c|%c|\n", data[0], data[1], data[2]);
}
