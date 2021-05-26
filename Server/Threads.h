#ifndef THREADS_H
#define THREADS_H

#include "server.h"

#define NUM_OF_THREADS 4

void mutexAndSemaphoresReset();
void resetThreadHandles();

HANDLE log_mutex;
HANDLE mutex1;
HANDLE mutex2;
HANDLE game_start_sem;
HANDLE requestMutex;
HANDLE killSem;
HANDLE restartSem;
HANDLE ThreadHandles[NUM_OF_THREADS];

#endif