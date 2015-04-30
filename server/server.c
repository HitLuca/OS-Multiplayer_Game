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

void parseMessage(char *rawMessage);
void encapsuleMessage(char *message);

void* authorizationThread(void* arg);


int main(int argc,char **argv)
{
	//Check se il server è gia avviato
	if (mkfifo(SERVER_AUTHORIZATION_FIFO,FILE_MODE)!=0)
	{
		printf("Server gia presente\n");
		return 0;
	}
	else
	{
		printf("Sono l'unico server attivo\n");

		//Creo il thread con la parte di autorizzazione
		pthread_t authorization;
		pthread_create (&authorization, NULL, &authorizationThread, NULL);

		mkfifo(SERVER_ANSWER_FIFO,FILE_MODE);
		int serverAnswerFIFO = open(SERVER_ANSWER_FIFO,O_RDWR);

		//Il server legge le risposte da serverAnswerFIFO
		char message[MAX_MESSAGE_SIZE];
		printf("answThread: In lettura:\n");
		while (1) 
		{
			read(serverAnswerFIFO,message,MAX_MESSAGE_SIZE);
			parseMessage(message);
			printf("answThread: Ho ricevuto %s nella FIFO risposte\n", message);
		}
		return 0;
	}
}

void parseMessage(char *rawMessage)
{
	*strchr(rawMessage,'!')='\0';
}

void encapsuleMessage(char *message)
{
	message[strlen(message)]='!';
}

void* authorizationThread(void* arg)
{
	//Avvio il monitoraggio delle autorizzazioni
	int serverAuthFIFO = open(SERVER_AUTHORIZATION_FIFO,O_RDWR);
	char clientMessage[MAX_MESSAGE_SIZE];
	char serverMessage[MAX_MESSAGE_SIZE];

	int maxPlayers=1;
	int actualPlayers=0;

	while(1)
	{
		read(serverAuthFIFO,clientMessage,MAX_MESSAGE_SIZE);
		parseMessage(clientMessage);
		printf("authThread: %s mi ha contattato\n", clientMessage);

		//Creo il collegamento alla FIFO del client
		char fifoPath [MAX_FIFO_NAME_SIZE] = CLIENT_MESSAGE_FIFO;
		strcat(fifoPath,clientMessage);
		int clientMessageFIFO = open(fifoPath,O_RDWR);
		
		if (actualPlayers<maxPlayers)
		{
			strcpy(serverMessage, "1");
			encapsuleMessage(serverMessage);
			actualPlayers++;
		}
		else
		{
			strcpy(serverMessage, "0");
			encapsuleMessage(serverMessage);
		}

		write(clientMessageFIFO, serverMessage, strlen(serverMessage));
	
	}

	printf("authThread: Raggiunto il numero massimo di giocatori, termino");
}