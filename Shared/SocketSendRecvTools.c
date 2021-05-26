#include "SocketSendRecvTools.h"

#include <stdio.h>
#include <string.h>

#define MAX_MESSAGE_LENGTH 100

TransferResult_t SendString(const char *Str, SOCKET sd)
{//Sends a string to a destination SOCKET.
	int i = 0;
	while (Str[i] != '\n')
	{
		if(send(sd, &Str[i], 1, 0)!=1)
		{
			return TRNS_FAILED;
		}
		i++;
	}
	if (send(sd, &Str[i], 1, 0) == SOCKET_ERROR)
	{
		return TRNS_FAILED;
	}
	return TRNS_SUCCEEDED;
}


TransferResult_t ReceiveString(char** OutputStrPtr, SOCKET sd)
{//Allocates memory and receives a string from a source SOCKET, the string is copied to the 
	//allocated memory.
	char *message = (char *)malloc(sizeof(char)*MAX_MESSAGE_LENGTH);
	char c[1];
	int i = 0;
	do
	{
		if (recv(sd, c, 1, 0)!=1)
		{
			return TRNS_FAILED;
		}
		message[i] = *c;
		i++;
	} while (*c != '\n');
	message[i] = '\0';
	*OutputStrPtr = message;
	return TRNS_SUCCEEDED;
}
