#ifndef COMMONLIB_H
#define COMMONLIB_H

//Definizione costanti comuni
#define SERVER_ANSWER_FIFO "/tmp/SANSF"
#define SERVER_AUTHORIZATION_FIFO "/tmp/SAUTHF"
#define CLIENT_MESSAGE_FIFO "/tmp/CMF"
#define MAX_MESSAGE_SIZE 30000
#define MAX_FIFO_NAME_SIZE 100
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define MAX_QID_SIZE 4
#define MAX_QUESTION_SIZE 2900
#define POINT_SIZE 4

#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_RESET   "\x1b[0m"

typedef enum {
	GAME, 
	INFO, 
	ERROR, 
	AUTH,
	DEFAULT
} tags;

//Struct contenente i messaggi inviati tra Client e Server
typedef struct {
	char** parameters;
	int parameterCount;
} Message;

//Struct contenente le domande, diversa da quella lato server che contiene anche la risposta <--------------SERVE IN COMMONLIB??? (SERVERLIB MAYBE?)
typedef struct {
	char text[MAX_QUESTION_SIZE];
	char id[MAX_QID_SIZE];
} Question;

//Buffer per le stringhe da stampare 
char stringBuffer[2000];

//Funzioni comuni
Message* parseMessage(char *message);
Message** parseMessages(char *messages,int size);
void printScreen(int test, tags tag, char* message);
void printFile(FILE* file, tags tag, char* message);
void printTitle(int color);

#endif
