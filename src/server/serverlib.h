#ifndef SERVERLIB_H
#define SERVERLIB_H

#include "../common/commonlib.h"

#define MAX_COMMAND_SIZE 100
#define MAX_CLIENTS 10
#define MAX_PID_LENGTH 15
#define MAX_PARAMETERS_NUMBER 6
#define QUESTION_ID 8

typedef struct {
	char* name;
	char* pid;
	int points;
} ClientData;

int connectedClientsNumber;
int clientsMaxNumber;
Question currentQuestion;
ClientData** clientData;

void* authorizationThread(void* arg);
void* bashThread(void*arg);
void* senderThread(void*arg);
int checkClientRequest(Message *message);
void initializeClientData();
void connectNewClient(int id,char* name,char* pid);
char* authAcceptMessage(int id);
char* authRejectMessage(int error);

#endif