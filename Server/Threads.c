#include "Threads.h"

void mutexAndSemaphoresReset()
{//Resets the mutex and semaphores for necessary for the server
	if (restartSem != NULL)
		if (!CloseHandle(restartSem))
		{
			fprintf(log, "CLoseHandle failed, Exiting\n");
			exit(1);
		}
	restartSem = CreateSemaphore(NULL, 0, 1, NULL);
	if (restartSem == NULL)
	{
		fprintf(log, "CreateSemaphore failed, Exiting.\n");
		exit(1);
	}
	if (requestMutex != NULL)
	{
		if (!CloseHandle(requestMutex))
		{
			fprintf(log, "CLoseHandle failed, Exiting\n");
			exit(1);
		}
	}
	requestMutex = CreateMutex(NULL, FALSE, NULL);
	if (requestMutex == NULL) 
	{
		fprintf(log, "CreateMutex failed, Exiting.\n");
		exit(1);
	}
	if (mutex1 != NULL)
		if (!CloseHandle(mutex1)) 
		{
			fprintf(log, "CLoseHandle failed, Exiting\n");
			exit(1);
		}
	mutex1 = CreateMutex(NULL, FALSE, NULL);
	if (mutex1 == NULL) 
	{
		fprintf(log, "CreateMutex failed, Exiting.\n");
		exit(1);
	}
	if (mutex2 != NULL)
		if (!CloseHandle(mutex2)) 
		{
			fprintf(log, "CLoseHandle failed, Exiting\n");
			exit(1);
		}
	mutex2 = CreateMutex(NULL, FALSE, NULL);
	if (mutex2 == NULL) 
	{
		fprintf(log, "CreateMutex failed, Exiting.\n");
		exit(1);
	}
	if (game_start_sem != NULL)
		if (!CloseHandle(game_start_sem)) 
		{
			fprintf(log, "CLoseHandle failed, Exiting\n");
			exit(1);
		}
	game_start_sem = CreateSemaphore(NULL, 1, 2, NULL);
	if (game_start_sem == NULL)
	{
		fprintf(log, "CreateSemaphore failed, Exiting.\n");
		exit(1);
	}
	if (killSem != NULL)
		if (!CloseHandle(killSem)) 
		{
			fprintf(log, "CLoseHandle failed, Exiting\n");
			exit(1);
		}
	killSem = CreateSemaphore(NULL, 0, 1, NULL);
	if (killSem == NULL) 
	{
		fprintf(log, "CreateSemaphore failed, Exiting.\n");
		exit(1);
	}
}

void resetThreadHandles()
{//Resets the thread handles.
	for (int i = 0; i < NUM_OF_THREADS; i++)
		ThreadHandles[i] = NULL;
}
