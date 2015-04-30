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
#define MAX_COMMAND_SIZE 100
#define MAX_CLIENTS 10
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)


void parseMessage(char *rawMessage);
void encapsuleMessage(char *message);

void* authorizationThread(void* arg);
void* bashThread(void*arg);
void* senderThread(void*arg);

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
		//Creo il thread con la parte di autorizzazione
		pthread_t authorization;
		pthread_create (&authorization, NULL, &authorizationThread, NULL);


		//Creo il thread per i comandi utente
		pthread_t bash;
		pthread_create (&bash, NULL, &bashThread, NULL);

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

	//Array di argomenti da passare al sender (nomeFIFOclient e messaggio)
	char argList[2][MAX_MESSAGE_SIZE];

	//Lista dei client con cui comunica il server
	char clientList[MAX_CLIENTS][MAX_MESSAGE_SIZE];

	int actualClients=0;

	while(1)
	{
		read(serverAuthFIFO,clientMessage,MAX_MESSAGE_SIZE);
		parseMessage(clientMessage);

		printf("authThread: %s mi ha contattato\n", clientMessage);
		
		if (actualClients<MAX_CLIENTS)
		{
			//Aggiungo il client alla lista
			strcpy(clientList[actualClients],clientMessage);
			printf("authThread: aggiorno la lista dei client\n");

			strcpy(serverMessage, "1");
			encapsuleMessage(serverMessage);
			actualClients++;
		}
		else
		{
			strcpy(serverMessage, "0");
			encapsuleMessage(serverMessage);
		}

		//Preparo gli argomenti del thread
		strcpy(argList[0][0], clientMessage);
		strcpy(argList[1][0], serverMessage);

		pthread_t sender;
		pthread_create (&sender, NULL, &senderThread, &argList);
	}
}

void* senderThread(void*arg)
{
	/*printf("%s", (char*)arg[0]);
	char arg1[MAX_MESSAGE_SIZE];
	char arg2[MAX_MESSAGE_SIZE];
	//strcpy(arg1, (char**)arg[0]);
	//strcpy(arg2, (char*)arg[1]);
	printf("arg1new: %s\n", arg1);
	printf("arg2new: %s\n", arg2);

	//Creo il collegamento alla FIFO del client
	char fifoPath [MAX_FIFO_NAME_SIZE] = CLIENT_MESSAGE_FIFO;
	strcat(fifoPath,arg1);
	int clientMessageFIFO = open(fifoPath,O_RDWR);
	write(clientMessageFIFO, arg2, strlen(arg2));*/
}

void* bashThread(void*arg)
{
	char comando[MAX_COMMAND_SIZE];
	printf("\e[1;1H\e[2J");
	printf("Benvenuto nel terminale utente!\nDigita help per una lista dei comandi\n");
	while(1)
	{
		printf(">");
		scanf("%s", comando);
		if (strcmp(comando, "help")==0)
		{
			printf("Lista comandi utente:\n");
			printf("help: Richiama questo messaggio di aiuto\n");
			printf("kick <utente>: Kick di <utente> dalla partita\n");
			printf("question <question>: Invia una nuova domanda a tutti gli utenti\n");
			printf("clear: Pulisce la schermata corrente\n");
		}
		else if (strcmp(comando, "clear")==0)
		{
			printf("\e[1;1H\e[2J");
		}
	}
}