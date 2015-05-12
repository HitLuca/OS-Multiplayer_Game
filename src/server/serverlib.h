#ifndef SERVERLIB_H
#define SERVERLIB_H

#include "../common/commonlib.h"

#define MAX_COMMAND_SIZE 100
#define CLIENTS_MAX_NUMBER 10
#define WIN_POINTS 5
#define MAX_PID_LENGTH 15
#define MAX_PARAMETERS_NUMBER 6
#define QUESTION_ID 8

typedef struct {
	char* name;
	int fifoID;
	int points;
} ClientData;

typedef struct {
	Question* question;
	char* answer;
}QuestionData;

int connectedClientsNumber;
int clientsMaxNumber;
int winPoints;
int currentQuestion;
QuestionData questions[QUESTION_ID];
ClientData** clientData;

void* authorizationThread(void* arg);
void* bashThread(void*arg);
int checkClientRequest(Message *message);
void initializeClientData();
void connectNewClient(int id,char* name,int fifoID);
char* authAcceptMessage(int id);
char* authRejectMessage(int error);
int checkAnswer(Message* message);
char* buildResult(Message* message, ClientData* player, int cwt);
ClientData* getSender(Message* message);
void sendResponse(int fifoID, char* response);
#endif