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
#define MAX_PID_LENGTH 15
#define MAX_PARAMETERS_NUMBER 6
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

//Array di argomenti da passare al sender (nomeFIFOclient e messaggio)
//METTERE A POSTO WTF

typedef struct {
	char** parameters;
	int parameterCount;
} Message;

void* authorizationThread(void* arg);
void* bashThread(void*arg);
void* senderThread(void*arg);

Message* parseMessage(char *message);


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
		//printf("answThread: In lettura:\n");
		while (1) 
		{
			read(serverAnswerFIFO,message,MAX_MESSAGE_SIZE);
			printf("answThread: Ho ricevuto %s nella FIFO risposte\n", message);
		}
		return 0;
	}
}

void* authorizationThread(void* arg)
{
	//Avvio il monitoraggio delle autorizzazioni
	int serverAuthFIFO = open(SERVER_AUTHORIZATION_FIFO,O_RDWR);
	char clientMessage[MAX_MESSAGE_SIZE];
	char serverMessage[MAX_MESSAGE_SIZE];
	

	//Lista dei client con cui comunica il server TODO VA GLOBALE E PROTETTA CON UN SEMAFORO
	char clientList[MAX_CLIENTS][MAX_MESSAGE_SIZE];

	int actualClients=0;

	while(1)
	{
		read(serverAuthFIFO,clientMessage,MAX_MESSAGE_SIZE);
		printf("authThread: Ho ricevuto %s \n",clientMessage);
		
		if (actualClients<MAX_CLIENTS)
		{
			//Aggiungo il client alla lista TODO PREVEDERE L'ASSEGNAZIONE E PASSAGGO DI USERNAME
			/*char *separator = strchr(clientMessage,'|');
			*separator ='\0';
			char *clientPid = (char*)malloc(sizeof(char)*(strlen(clientMessage)+1));
			strcpy(clientPid,clientMessage);
			separator++;
			char *username = (char*)malloc(sizeof(char)*(strlen(separator)+1));
			strcpy(username,separator);
			printf("hai il pid %s e l'username %s\n",clientPid,username);*/
			
			Message *message = parseMessage(clientMessage);

			//Definire e implementare il protocollo di risposta
			strcpy(serverMessage, "1");
			actualClients++;
		}
		else
		{
			strcpy(serverMessage, "0");
		}

		//Preparo gli argomenti del thread
		/*char** argList = malloc(2 * sizeof(char*));
		argList[0]=(char *)malloc(MAX_MESSAGE_SIZE * sizeof(char));
		argList[1]=(char *)malloc(MAX_MESSAGE_SIZE * sizeof(char));
		
		strcpy(argList[0], clientMessage);
		strcpy(argList[1], serverMessage);


		//TOCHECK non so se abbia senso creare un altro thread per questo... Parliamone
		pthread_t sender;
		pthread_create (&sender, NULL, &senderThread,  (void*)argList);*/
		
		char fifoPath [MAX_FIFO_NAME_SIZE];
		strcpy(fifoPath,CLIENT_MESSAGE_FIFO);
		strcat(fifoPath,clientMessage);
		
		char answer[MAX_MESSAGE_SIZE]; //TODO definire protocollo risposta autorizzazione
		strcpy(answer,"1");
		
		//scrivo la risposta al client
		int clientMessageFIFO = open(fifoPath,O_RDWR);
		write(clientMessageFIFO,answer, strlen(answer)+1);
	}
}

void* senderThread(void*arg)
{
	//Creo il collegamento alla FIFO del client
	char fifoPath [MAX_FIFO_NAME_SIZE];
	strcpy(fifoPath,CLIENT_MESSAGE_FIFO);
	strcat(fifoPath,((char**)arg)[0]);
	printf("Scrivo sulla FIFO: %s\n",fifoPath);

	int clientMessageFIFO = open(fifoPath,O_RDWR);
	write(clientMessageFIFO,((char**)arg)[1], strlen(((char**)arg)[1])+1);
	free(arg);
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
			printf("list: Stampa la lista degli utenti connessi\n");
			printf("clear: Pulisce la schermata corrente\n");
		}
		else if (strcmp(comando, "clear")==0)
		{
			printf("\e[1;1H\e[2J");
		}
		else
		{
			printf("%s:Comando non riconosciuto\n",comando);
		}
	}
}

Message* parseMessage(char *message)
{
	int count = 0;
	char ** p = (char**)(malloc(sizeof(char*)*MAX_PARAMETERS_NUMBER));
	int i;
	char *last=message;
	char *separator = strchr(message,'|');
	while(separator!=NULL)
	{
		separator='\0';
		char* parameter = (char*)malloc(separator-last+sizeof(char));
		strcpy(parameter,last);
		p[count] = parameter;
		count++;
		last=separator+1;
		separator=strchr(separator+1,'|');
	}
	Message * m = (Message*) malloc(sizeof(Message));
	m->parameters = p;
	m->parameterCount = count-1;
	return m;
}
