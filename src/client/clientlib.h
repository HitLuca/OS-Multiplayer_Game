#ifndef CLIENTLIB_H
#define CLIENTLIB_H

#include "../common/commonlib.h"

#define MAX_USERNAME_LENGHT 20
#define MIN_USERNAME_LENGHT 1
#define MAX_CONCURRENT_MESSAGES 8

typedef struct {
	char* name;
	char* id;
	char* points;
} ClientData;

int serverAuthFIFO;
int inMessageFIFO;
char messageFIFOName[MAX_FIFO_NAME_SIZE];
Question currentQuestion;
ClientData* clientData;
int serverAnswerFIFO;
pthread_mutex_t mutex;
int waitingForUserInput;
int connected;
int newQuestion;
int endGame;

void* userInput(void* arg);
int validateUsername(char* username);
char* authRequestMessage(char* pid,char *name);
int checkServerAuthResponse(Message* message);
void initializeClientData(Message *message);
void sendResponse(int serverAnswerFIFO, char* answer);
void initializeQuestion(Message *message);
void setNewQuestion(Message *message);
void deallocResources();
void handler();

#endif
