#ifndef COMMONLIB_H
#define COMMONLIB_H

#define SERVER_ANSWER_FIFO "/tmp/SANSF"
#define SERVER_AUTHORIZATION_FIFO "/tmp/SAUTHF"
#define CLIENT_MESSAGE_FIFO "/tmp/CMF"
#define MAX_MESSAGE_SIZE 1000
#define MAX_FIFO_NAME_SIZE 100
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define MAX_QID_SIZE 4
#define MAX_QUESTION_SIZE 30
#define POINT_SIZE 4

typedef struct {
	char** parameters;
	int parameterCount;
} Message;

typedef struct {
	char text[MAX_QUESTION_SIZE];
	char id[MAX_QID_SIZE];
} Question;

Message* parseMessage(char *message);
#endif