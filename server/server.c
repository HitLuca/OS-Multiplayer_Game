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
#define MAX_QUESTION_SIZE 30
#define MAX_QID_SIZE 4
#define QUESTION_ID 8
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)


//Array di argomenti da passare al sender (nomeFIFOclient e messaggio)
//METTERE A POSTO WTF

typedef struct {
	char** parameters;
	int parameterCount;
} Message;

typedef struct {
	char* name;
	char* pid;
	int points;
} ClientData;

typedef struct {
	char text[MAX_QUESTION_SIZE];
	char id[MAX_QID_SIZE];
} Question;

void* authorizationThread(void* arg);
void* bashThread(void*arg);
void* senderThread(void*arg);
int checkClientRequest(Message *message);
void initializeClientData();
void connectNewClient(int id,char* name,char* pid);
char* authAcceptMessage(int id);
char* authRejectMessage(int error);

Message* parseMessage(char *message);

int connectedClientsNumber;
int clientsMaxNumber;
Question currentQuestion;

ClientData** clientData;

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

		
		connectedClientsNumber=0;
		clientsMaxNumber=10; //TODO imposta il massimo da parametro
		initializeClientData();
		currentQuestion.id="0";
		strcpy(currentQuestion.text,"3 + 2 = ?");

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
	char fifoPath [MAX_FIFO_NAME_SIZE];
	
	char* answer;
	
	while(1)
	{
		strcpy(fifoPath,CLIENT_MESSAGE_FIFO);
		read(serverAuthFIFO,clientMessage,MAX_MESSAGE_SIZE);
		printf("authThread: Ho ricevuto %s \n",clientMessage);
		Message *message = parseMessage(clientMessage);
		
		strcat(fifoPath,message->parameters[0]);
		
		printf(" %s ha effettuato una richiesta di connessione\n",message->parameters[1]);
		
		//apro la fifo del client
		int clientMessageFIFO = open(fifoPath,O_RDWR);
		write(clientMessageFIFO,"1schifoso",strlen("0schifoso"));
		
		int id=checkClientAuthRequest(message);
		
		if(id>=0)
		{
			connectNewClient(id,message->parameters[1],message->parameters[0]);
			answer=authAcceptMessage(id);
		}
		else
		{
			answer=authRejectMessage(id);
		}
		write(clientMessageFIFO,answer, strlen(answer)+1);
		free(message);
		free(answer);
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
	//conto i parametri
	int parameterCount = 1;
	char *i = strchr(message,'|');
	while(i!=NULL)
	{
		parameterCount++;
		i++;
		i=strchr(i,'|');
	}
	//alloco lo spazio necessario
	char ** p = (char**)(malloc(sizeof(char*)*parameterCount));
	char *last=message;
	char *separator = strchr(message,'|');
	int count = 0;
	
	//separo e salvo i parametri
	while(separator!=NULL)
	{
		*separator='\0';
		char* parameter = (char*)malloc((separator-last+1)*sizeof(char));
		strcpy(parameter,last);
		p[count] = parameter;
		count++;
		last=separator+1;
		separator=strchr(separator+1,'|');
	}
	char* parameter = (char*)malloc((strlen(last)+1)*sizeof(char));
	strcpy(parameter,last);
	p[count] = parameter;
	
	//construisco la strurrura del messaggio e lo ritorno
	Message * m = (Message*) malloc(sizeof(Message));
	m->parameters = p;
	m->parameterCount = parameterCount;
	return m;
}

int checkClientAuthRequest(Message *message)
{
	if(message->parameterCount!=2)
	{
		return -2; //WRONG PARAMETER COUNT
	}
	else
	{
		if (connectedClientsNumber<clientsMaxNumber)
		{
			int i;
			for(i=0;i<clientsMaxNumber;i++)
			{
				if(clientData[i]!=NULL)
				{
					if(strcmp(clientData[i]->name,message->parameters[1])==0)
					{
						return -4; //NAME ALREADY IN USE
					}
				}
			}
			for(i=0;i<clientsMaxNumber;i++)
			{
				if(clientData[i]==NULL)
				{
					/*clientNames[i]=(char*)malloc((srtlen(message->parameters[1])+1)*sizeof(char));
					strcpy(clientNames[i],message->parameters[1]);
					connectedClientsNumber++;*/
					return i;
				}
			}
			
		}
		else
		{
			return -3; //SERVER FULL
		}
	}
}

void initializeClientData()
{
	clientData = (ClientData**)malloc(sizeof(ClientData*)*clientsMaxNumber);
	int i;
	for(i=0;i<clientsMaxNumber;i++)
	{
		clientData[i]=NULL;
	}
}

void connectNewClient(int id,char* name,char* pid)
{
	clientData[id]=(ClientData*)malloc(sizeof(ClientData));
	clientData[id]->name=(char*)malloc(sizeof(char)*(strlen(name)+1));
	strcpy(clientData[id]->name,name);
	clientData[id]->pid=(char*)malloc(sizeof(char)*(strlen(pid)+1));
	strcpy(clientData[id]->pid,pid);
	clientData[id]->points=clientsMaxNumber-connectedClientsNumber;
	connectedClientsNumber++;
	printf("Ho allocato lo slot %d con il client %s che ha il pid %s e parte con %d punti, sono rimasti %d posti liberi\n",id,clientData[id]->name,clientData[id]->pid,clientData[id]->points,clientsMaxNumber-connectedClientsNumber);
}

char* authAcceptMessage(int id)
{
	char idc[3];
	char points[10];
	sprintf(idc,"%d",id);
	sprintf(points,"%d",clientData[id]->points);
	char *answer = (char*)malloc(sizeof(char)*MAX_MESSAGE_SIZE);
	strcpy(answer,"A|");
	strcat(answer,idc);
	strcat(answer,"|");
	strcat(answer,currentQuestion.text);
	strcat(answer,"|");
	strcat(answer,currentQuestion.id);
	strcat(answer,"|");
	strcat(answer,points);
	return answer;
}

char* authRejectMessage(int error)
{
	char errors[4];
	sprintf(errors,"%d",error);
	char *answer = (char*)malloc(sizeof(char)*MAX_MESSAGE_SIZE);
	strcpy(answer,"A|");
	strcat(answer,errors);
	return answer;
}
