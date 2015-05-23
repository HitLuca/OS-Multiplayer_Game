#ifndef SERVERLIB_H
#define SERVERLIB_H

#include "../common/commonlib.h"

//Definizioni costanti
#define MAX_COMMAND_SIZE 100
#define CLIENTS_MAX_NUMBER 10
#define WIN_POINTS 20
#define MAX_PID_LENGTH 15
#define MAX_PARAMETERS_NUMBER 6
#define QUESTION_ID 8
#define MAX_QUESTION_NUM 100
#define BASH ""

//Struct contenente i dati dei client necessari a contattarli e operare su di essi
typedef struct {
	char* name;
	int fifoID;
	int points;
} ClientData;

//Struct contenente i dati della domanda corrente
typedef struct {
	Question* question;
	char* answer;
}QuestionData;

//Struct contenente i comandi bash
typedef struct {
	char* operation;
	char** parameters;
	int parameterCount;
}Command;

//Variabili globali
int serverAuthFIFO;
int serverAnswerFIFO;
int connectedClientsNumber;
int clientsMaxNumber;
int winPoints;
int currentQuestion;
QuestionData questions[QUESTION_ID];
ClientData** clientData;
int testRun;
int colorRun;
FILE* testFile;
FILE* logFile;
pthread_t bash;
pthread_mutex_t printMutex;
pthread_mutex_t data;
int waiting;

//Funzioni lato server
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
void InitializeQuestions();
void GenerateNewQuestion();
void BroadcastQuestion();
void disconnectClient(int id);
void handler ();
void broadcastServerClosed();
void kick(int id);
Command* parseCommand(char* command);
void listCommand();
void sendCustomizedQuestion(char* question,char* answer);
void broadcastConnection(int id,char* name);
void broadcastDisonnection(int id);
void endGame(ClientData* winner);
void broadcastEndGame();
void broadcastRank(ClientData* best);
void notifyAll(char* message);
void notifyPlayers(char* message,char ** players,int playerNumber);
void kickAll();
void kickPlayers(char ** players,int playerNumber);
void print(tags tag, char* message);
void* waitingThread(void* arg);
void deallocResources();
void rankCommand();

#endif
