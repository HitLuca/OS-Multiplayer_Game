#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#define SERVER_ANSWER_FIFO "/tmp/SANSF"
#define SERVER_AUTHORIZATION_FIFO "/tmp/SAUTHF"
#define CLIENT_MESSAGE_FIFO "/tmp/CMF"
#define MAX_MESSAGE_SIZE 1000
#define MAX_FIFO_NAME_SIZE 100
#define MAX_USERNAME_LENGHT 20
#define MIN_USERNAME_LENGHT 1
#define MAX_QUESTION_SIZE 30
#define MAX_QID_SIZE 4

#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

typedef struct {
	char** parameters;
	int parameterCount;
} Message;

typedef struct {
	char text[MAX_QUESTION_SIZE];
	char id[MAX_QID_SIZE];
} Question;

typedef struct {
	char* name;
	char* id;
	char* points;
} ClientData;

void* userInput(void* arg);
int validateUsername(char* username);
char* authRequestMessage(char* pid,char *name);
int checkServerAuthResponse(Message* message);
void initializeClientData(Message *message);

Message* parseMessage(char *message);

int serverAuthFIFO;
Question currentQuestion;
ClientData* clientData;

int main()
{
	//provo ad aprire la fifo di autorizzazione del server
	serverAuthFIFO = open(SERVER_AUTHORIZATION_FIFO,O_WRONLY);
	
	
	if(serverAuthFIFO==-1 )	//se la fifo non è presente significa che non vi è nessun server
	{
		printf("Server non presente\n");
		return 1;
	}
	else	
	{
		//genero il nome della mia fifo univocamente utilizzando il PID e la parso
		char pid[100];
		sprintf(pid,"%d",getpid());
		char messageFIFOName[MAX_FIFO_NAME_SIZE] = CLIENT_MESSAGE_FIFO;
		strcat(messageFIFOName,pid);
		
		mkfifo(messageFIFOName,FILE_MODE);
		int inMessageFIFO = open(messageFIFOName,O_RDWR);
	
		//se l'apertura non va a buon fine stampo un errore
		if(inMessageFIFO==-1)
		{
		printf("Errore di apertura FIFO\n");
		return 0;
		}
		
		//chiedo all'utente di inserire un username
		char username[MAX_USERNAME_LENGHT];
		int correctUsername=-1;
		while(correctUsername==-1)
		{
			printf("Inserisci il tuo username> ");
			scanf("%s",username);
			correctUsername = validateUsername(username);
		}
		
		//mando un messaggio al server richiedendo l'autorizzazione e passandogli il mio pid e il mio username
		char *message = authRequestMessage(pid,username);
		
		
		if(write(serverAuthFIFO,message,strlen(message)+1))
		{
			//aspetto una risposta
			char answerBuffer[MAX_MESSAGE_SIZE];
			read(inMessageFIFO,answerBuffer,MAX_MESSAGE_SIZE);
			printf("%s\n",answerBuffer);
			Message *answer = parseMessage(answerBuffer);
			
			int answerResult = checkServerAuthResponse(answer);
			
			if(answerResult<0)
			{
				
				printf("Errore %d\n",answerResult); //TODO gestire i codici di errore
				return answerResult;
			}
			else
			{
				//provo ad aprire la FIFO delle risposte alle domande da inviare al server
				int serverAnswerFIFO = open(SERVER_ANSWER_FIFO,O_RDWR);
				
				if(serverAnswerFIFO == -1)
				{
					printf("Errore di connessione al server\n");
					return 0;
				}
				
				printf("Connessione Riuscita\n");
				
				//inizializzo le variabili
				clientData = (ClientData*) malloc(sizeof(ClientData));
				clientData->name=username;
				initializeClientData(answer);
				
				printf("Il server mi ha assegnato l'id %s e %s punti\n",clientData->id,clientData->points);

				//Componenti del thread bash
				pthread_t bash;
				char arg[MAX_MESSAGE_SIZE];

				strcpy(arg, "[attesa domanda dal server]");
				
				pthread_create (&bash, NULL, &userInput, arg);

				while (1)
				{
					printf("Entro nel loop\n");	
					
					//mi metto in ascolto
					char message[MAX_MESSAGE_SIZE];
					printf("mi metto in ascolto\n");
					if(read(inMessageFIFO,message,MAX_MESSAGE_SIZE))
					{
						printf("Ho ricevuto: %s \n",message);
						//se ricevo il messaggio di kick mi chiudo
						if(message[0]=='k')
						{
							printf("Kicked by server\n");
							return 0;
						}
						else 
						{
							//Allora ho una domanda e devo mostrarla in console
							//Prima uccido il processo
							if (pthread_cancel(bash)!=0)
							{
								printf("Impossibile terminare il thread bash :(\n");
							}

							strcpy(arg, message);
							printf("%s\n%s", arg,message);

							//creo un nuovo thread a cui associo l'input dell'utente
							pthread_create (&bash, NULL, &userInput, &arg);


						}
					}
					else // altrimenti se ricevo un errore mi chiudo preventivamente
					{
						printf("Errore in lettura messaggio server\n");
						return 0;
					}
				}
			}
		} else {
			printf("Errore richiesta autorizzazione\n");
		}
	}
	return 0;
}

void* userInput(void* arg)
{
	int serverAnswerFIFO = open(SERVER_ANSWER_FIFO,O_RDWR);
	printf("La domanda è: %s\n", (char*) arg);
	printf("Scrivi la tua risposta\n");
	char input[MAX_MESSAGE_SIZE];

	//Il client invia le risposte ad serverAnswerFIFO
	while(1)
	{
		printf(">");
		scanf("%s",input);
		write(serverAnswerFIFO,input,strlen(input)+1);
	}
}

int validateUsername(char* username)
{
	printf("hai scelto %s\n",username);
	if(strlen(username)<MIN_USERNAME_LENGHT)
	{
		printf("Errore: username troppo corto\n");
		return -1;
	}
	else if(strlen(username)>MAX_USERNAME_LENGHT)
	{
		printf("Errore: username troppo lungo\n");
		return -1;
	}
	else
	{
		int i;
		for(i=0;i<strlen(username);i++)
		{
			if(!isalnum(username[i]))
			{
				printf("Errore: carattere %c non valido\n",username[i]);
				return -1;
			}
		}
		return 0;
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

char* authRequestMessage(char* pid,char* name)
{
	char *message = (char*)malloc(sizeof(char)*(strlen(pid)+strlen(name)+2));
	strcpy(message,pid);
	strcat(message,"|");
	strcat(message,name);
	return message;
}

int checkServerAuthResponse(Message* message){
	if(strcmp(message->parameters[0],"A")==0)
	{
		if(message->parameterCount==2)
		{
			return atoi(message->parameters[1]);
		}
		else if(message->parameterCount==5)
		{
			return 0;
		}
	}
	return -1;
}

void initializeClientData(Message *message){
	clientData->id=(char*)malloc(sizeof(char)*(strlen(message->parameters[1])+1));
	strcpy(clientData->id,message->parameters[1]);
	clientData->points=(char*)malloc(sizeof(char)*(strlen(message->parameters[4])+1));
	strcpy(clientData->points,message->parameters[4]);
}

