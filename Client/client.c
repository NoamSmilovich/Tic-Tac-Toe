#include "client.h"

void main(int argc, char *argv[])
{
	ClientSetup(argv);

	char *name = argv[4];

	hThread[0] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SendDataThread, name, 0, NULL);
	hThread[1] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RecvDataThread, NULL, 0, NULL);
	hThread[2] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Interface, NULL, 0, NULL);

	if (hThread[0] == NULL || hThread[1] == NULL || hThread[2] == NULL)
	{
		fprintf(log, "Thread creation failed. Exiting.\n");
		fclose(log);
		exit(0);
	}

	if (WaitForMultipleObjects(3, hThread, FALSE, INFINITE) == WAIT_FAILED)
	{
		fprintf(log, "WaitForMultipleObjects failed. Exiting.\n");
		fclose(log);
		exit(0);
	}

	if (WaitForSingleObject(log_mutex, INFINITE) == WAIT_FAILED)
	{
		fprintf(log, "WaitForMultipleObjects failed. Exiting.\n");
		fclose(log);
		exit(0);
	}

	printf("Server disconnected. Exiting.\n");
	fprintf(log,"Server disconnected. Exiting.\n");
	
	if(TerminateThread(hThread[0], 0x555)==0)
	{
		fprintf(log, "Thread termination failed, exiting.\n");
		fclose(log);
		exit(0);
	}
	if(TerminateThread(hThread[1], 0x555)==0)
	{
		fprintf(log, "Thread termination failed, exiting.\n");
		fclose(log);
		exit(0);
	}
	if(TerminateThread(hThread[2], 0x555)==0)
	{
		fprintf(log, "Thread termination failed, exiting.\n");
		fclose(log);
		exit(0);
	}

	if (ReleaseMutex(log_mutex) == 0)
	{
		fprintf(log, "ReleaseMutex failed, exiting.\n");
		fclose(log);
		exit(0);
	}
	if (CloseHandle(hThread[0]) == 0 || CloseHandle(hThread[1]) == 0 || CloseHandle(hThread[2]) == 0
		|| CloseHandle(SendingSem) == 0 || CloseHandle(InputSem) == 0 || CloseHandle(log_mutex) == 0)
	{
		fprintf(log, "CloseHandle failed, exiting.\n");
		fclose(log);
		exit(0);
	}
	if(shutdown(m_socket, 2)== SOCKET_ERROR)
	{
		fprintf(log, "shutdown failed, exiting.\n");
		fclose(log);
		exit(0);
	}
	if(closesocket(m_socket)!=0)
	{
		fprintf(log, "CloseHandle failed, exiting.\n");
		fclose(log);
		exit(0);
	}
	WSACleanup();
	fclose(log);
	return;
}

static DWORD RecvDataThread(void)
{//This thread is responsible for receving data from the server and decrypting it
	// and printing messages according to the decrypted message
	TransferResult_t RecvRes;
	while (!end)
	{
		int i = 0;
		char *AcceptedStr = NULL;
		RecvRes = ReceiveString(&AcceptedStr, m_socket);
		if (RecvRes == TRNS_FAILED)
		{
			printf("Socket error while trying to read data from socket\n");
			end = true;
			return 0x555;
		}
		else
		{
			WaitForSingleObject(log_mutex, INFINITE);
			fprintf(log, "Received from server: %s", AcceptedStr);
			ReleaseMutex(log_mutex);
			parseData(AcceptedStr);
		}
		free(AcceptedStr);
	}
	return 0;
}

static DWORD SendDataThread(char *name)
{// this thread is responsible for sending messages to the server, a message is sent only after
	// a command was input by the user and parsed, a semaphore was used to get the timing right.
	TransferResult_t SendRes;
	snprintf(toSend, SEND_STR_SIZE, "NEW_USER_REQUEST:%s\n\0", name);
	SendRes = SendString(toSend, m_socket);
	if (SendRes == TRNS_FAILED)
	{
		WaitForSingleObject(log_mutex, INFINITE);
		printf("Socket error while trying to write data to socket\n");
		fprintf(log,"Socket error while trying to write data to socket\n");
		ReleaseMutex(log_mutex);
		return 0x555;
	}
	while (!end)
	{		
		WaitForSingleObject(SendingSem, INFINITE);
		if (toSend[0] != '\0')
		{
			SendRes = SendString(toSend, m_socket);
			if (SendRes == TRNS_FAILED)
			{
				WaitForSingleObject(log_mutex, INFINITE);
				printf("Socket error while trying to write data to socket\n");
				fprintf(log, "Socket error while trying to write data to socket\n");
				ReleaseMutex(log_mutex);
				end = true;
				return 0x555;
			}
			WaitForSingleObject(log_mutex, INFINITE);
			fprintf(log, "Sent to server: %s",toSend);
			ReleaseMutex(log_mutex);
		}
		ReleaseSemaphore(InputSem, 1, NULL);
	}
	return 0;
}


void ClientSetup(char *argv[])
{//This function does some initial setup for the client program
	log_path = argv[1];
	ip = argv[2];
	port_str = argv[3];
	unsigned short port = (unsigned short)strtoul(argv[3], NULL, 0);
	
	log_mutex = CreateMutex(NULL, FALSE, NULL);
	if (log_mutex == NULL)
		exit(1);

	log = fopen(log_path, "w");
	if (log == NULL)
		exit(0);

	SOCKADDR_IN clientService;
	WSADATA wsaData; 
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR)
	{
		printf("Error at WSAStartup()\n");
		fprintf(log,"Error at WSAStartup()\n");
		fclose(log);
		exit(0);
	}
	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_socket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		fprintf(log, "Error at socket(): %ld\n", WSAGetLastError());
		fclose(log);
		WSACleanup();
		exit(0);
	}
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr(ip);
	clientService.sin_port = htons(port); 

	if (connect(m_socket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR) {
		printf("Failed to connect.\n");
		fprintf(log, "Failed to connect.\n");
		fclose(log);
		WSACleanup();
		exit(0);
	}

	SendingSem = CreateSemaphore(NULL, 0, 1, NULL);
	InputSem = CreateSemaphore(NULL, 1, 1, NULL);
	if(SendingSem==NULL || InputSem==NULL)
	{
		fprintf(log, "Semaphore creation failed, exiting.\n");
		fclose(log);
		exit(0);
	}

	end = false;
	toSend[0] = '\0';
}


static DWORD Interface(void)
{// This thread is responsible for the user interface, every user input is parsed and then a semaphore is
	//released which causes the SendDataThread to send the parsed input
	char input[SEND_STR_SIZE];
	input[0] = '\0';
	while (strcmp(input,"exit")!=0)
	{
		if (WaitForSingleObject(InputSem, INFINITE) == WAIT_FAILED)
		{
			fprintf(log, "WaitForSingleObject failed, exiting.\n");
			exit(0);
		}
		fgets(input,SEND_STR_SIZE,stdin);
		input[strlen(input) - 1] = '\0';
		parseUserInput(input, toSend);
		if(ReleaseSemaphore(SendingSem, 1, NULL)==0)
		{
			fprintf(log, "ReleaseSemaphore failed, exiting.\n");
			exit(0);
		}
	}
	return 0;
}

