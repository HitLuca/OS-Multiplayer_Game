#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>

#define SERVER_ANSWER_FIFO "/tmp/SANSF"
#define SERVER_AUTHORIZATION_FIFO "/tmp/SAUTHF"
#define CLIENT_MESSAGE_FIFO "/tmp/CMF"
#define MAX_MESSAGE_SIZE 1000
#define MAX_FIFO_NAME_SIZE 100

#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)


void parseMessage(char *rawMessage)
{
	*strchr(rawMessage,'!')='\0';
}

int main(int argc,char **argv)
{
	mkfifo(SERVER_ANSWER_FIFO,FILE_MODE);
	open(SERVER_ANSWER_FIFO,O_RDWR);
	if(mkfifo(SERVER_AUTHORIZATION_FIFO,FILE_MODE)==0)
	{
		int serverAuthFIFO = open(SERVER_AUTHORIZATION_FIFO,O_RDWR);
		char message[MAX_MESSAGE_SIZE];
		read(serverAuthFIFO,message,MAX_MESSAGE_SIZE);
		parseMessage(message);
		char fifoPath [MAX_FIFO_NAME_SIZE] = CLIENT_MESSAGE_FIFO;
		strcat(fifoPath,message);
		printf("Mando un messaggio a %s\n",message);
		int clientMessageFIFO = open(fifoPath,O_RDWR);
		write(clientMessageFIFO,"1schifoso",strlen("0schifoso"));
		
	}
	else
	{
		printf("Server gia presente\n");
		return 0;
	}
	return 0;
}
