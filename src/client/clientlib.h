//Autori
//Simonetto Luca 166540
//Federici Marco 165183
//Progetto Sistemi Operativi 1 2015
//Multiplayer Game

#ifndef CLIENTLIB_H
#define CLIENTLIB_H

#include "../common/commonlib.h"

//Definizione costanti
#define MAX_USERNAME_LENGHT 20
#define MIN_USERNAME_LENGHT 1
#define MAX_CONCURRENT_MESSAGES 8

//Struct contenente i dati del client, diversa da quella lato server
typedef struct {
	char* name;
	char* id;
	char* points;
} ClientData;

//Variabili globali
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
FILE* testFile;
FILE* logFile;
int testRun;
int colorRun;
char* username;

//Funzioni lato client
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
void* testInput(void* arg);
void printRanking(char** ranking,int size);

#endif
