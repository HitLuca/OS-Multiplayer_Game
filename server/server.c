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

#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)


void parseMessage(char *rawMessage)
{
	*strchr(rawMessage,'!')='\0';
}


int main(int argc,char **argv)
{
	if(mkfifo(SERVER_AUTHORIZATION_FIFO,FILE_MODE)==0)
	{
		int serverAuthFIFO = open(SERVER_AUTHORIZATION_FIFO,O_RDWR);
		char message[MAX_MESSAGE_SIZE];
		read(serverAuthFIFO,message,MAX_MESSAGE_SIZE);
		parseMessage(message);
		printf("%s\n",message);
	}
	else
	{
		printf("Server gia presente\n");
		return 0;
	}
	return 0;
}
