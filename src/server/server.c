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

#include "serverlib.h"

int main(int argc,char **argv) //server --max --win --test --color
{
	struct sigaction sa;
	sa.sa_handler = &handler;
	  
	sigaction (SIGKILL, &sa, NULL);
	sigaction (SIGABRT, &sa, NULL);
	sigaction (SIGQUIT, &sa, NULL);
	sigaction (SIGINT, &sa, NULL);
	sigaction (SIGSEGV, &sa, NULL);
	
	//Check se il server è gia avviato
	if (mkfifo(SERVER_AUTHORIZATION_FIFO,FILE_MODE)!=0)
	{
		print(ERROR, "Server gia presente\n");
		exit(EXIT_FAILURE);
	}
	else
	{
		//Setto le variabili iniziali
		connectedClientsNumber=0;
		currentQuestion=0;
		
		
		//Se la variabile max è presente la setto
		if (strcmp(argv[1],"0")!=0)
		{	
			clientsMaxNumber = atoi(argv[1]);
			//printf("ci sono %d clients al massimo\n",clientsMaxNumber);
		}
		else
		{
			clientsMaxNumber = CLIENTS_MAX_NUMBER;
		}

		//Se la variabile win è presente la setto
		if (strcmp(argv[2],"0")!=0)
		{	
			winPoints = atoi(argv[2]);
			//printf("massimo %d punti\n",winPoints);
		}
		else
		{
			winPoints = WIN_POINTS;
		}

		if (strcmp(argv[3],"0")!=0)
		{	
			testRun = 1;
		}
		else
		{
			testRun=0;
		}

		if (strcmp(argv[4],"0")!=0)
		{	
			colorRun = 1;
		}
		else
		{
			colorRun = 0;
		}
		
		//salvo il percorso attuale
		char currentPath[500];
		char* c;
	    char* past;
		strcpy(currentPath, argv[0]);
		c=currentPath;
		while(1)
		{
			past = c;
			c=strchr(c, '/');
			if (c==NULL)
			{
				break;
			}
			c++;
		}
		strcpy(past,"\0");
		
		//apro e creo il file di log;
		char logFilePath[500];
		strcpy(logFilePath,currentPath);
		strcat(logFilePath,"../../log/server/server.log");
		logFile=fopen(logFilePath,"w");
		if(logFile==NULL)
		{
			print(ERROR, "Errore creazione file di Log\n");
			exit(EXIT_FAILURE);
		}
		
		if(testRun==1)
		{
			char testFilePath[500];
			strcpy(testFilePath,currentPath);
			strcat(testFilePath,"../../assets/server/questions.test");

			testFile = fopen(testFilePath,"r");
			if(testFile==NULL)
			{
				print(ERROR, "Errore apertura file di test\n");
				exit(EXIT_FAILURE);
			}
		}

		if(testRun==0) //Faccio printf così non lo salvo nel logfile
		{
			printf("\e[1;1H\e[2J");
			printTitle(colorRun);
			if(colorRun)
			{
				printf("\t\t\t"COLOR_BLUE"SERVER\n\n"COLOR_RESET);
			}
			else
			{
				printf("\t\t\tSERVER\n\n");
			}
			printf("Benvenuto nel terminale utente!\n");
			printf("Digita help per una lista dei comandi\n");
			
		}	

		//Creo il thread con la parte di autorizzazione
		serverAuthFIFO = open(SERVER_AUTHORIZATION_FIFO,O_RDWR);
		pthread_t authorization;
		pthread_create (&authorization, NULL, &authorizationThread, NULL);


		//Inizializzazione delle strutture con i dati dei client e domande
		connectedClientsNumber=0;
		initializeClientData();

		currentQuestion=0;
		InitializeQuestions();
		GenerateNewQuestion();


		//Creo il thread per i comandi utente
		pthread_create (&bash, NULL, &bashThread, NULL);

		mkfifo(SERVER_ANSWER_FIFO,FILE_MODE);
		serverAnswerFIFO = open(SERVER_ANSWER_FIFO,O_RDWR);

		//Il server legge le risposte da serverAnswerFIFO
		char message[MAX_MESSAGE_SIZE*clientsMaxNumber];
		
		//se è una run di test lancio i clients
		if(testRun!=0)
		{
			if(fork()==0)
			{
				char launchClientsPath[500];
				char par1[20];
				char par2[20];
				strcpy(launchClientsPath,currentPath);
				strcat(launchClientsPath,"../test/launchClients");
				sprintf(par1,"%d",clientsMaxNumber);
				sprintf(par2,"%d",winPoints);
				execl(launchClientsPath,launchClientsPath,par1,par2,(char*)0);
			}
		
			pthread_t waiting;
			pthread_create (&waiting, NULL, &waitingThread, NULL);
			
		}

		//Ciclo continuamente per leggere i messaggi dei clients
		while (1) 
		{
			int size=read(serverAnswerFIFO,message,MAX_MESSAGE_SIZE*clientsMaxNumber);
			//printf("answThread: Ho ricevuto %s nella FIFO risposte\n", message);
			Message **messages=parseMessages(message,size);
			int i=0;
			while(messages[i]!=NULL)
			{
				Message* answer = messages[i];
				i++;

				sprintf(stringBuffer, "%s ha risposto %s alla domanda %s", clientData[atoi(answer->parameters[0])]->name, answer->parameters[2],  questions[atoi(answer->parameters[1])].question->text);
				print(GAME, stringBuffer);
				
				int result = checkAnswer(answer);				
				ClientData* client = getSender(answer);

				//Check della risposta del client
				if (result==1) //Risposta corretta
				{
					client->points++;
					sendResponse(client->fifoID, buildResult(answer, client, result));
					if(client->points>=winPoints)
					{
						//Partita finita
						endGame(client);
					}
					else
					{
						//Nuova domanda
						GenerateNewQuestion();
						BroadcastQuestion();
					}
				}
				else if (result==2) //Risposta errata
				{
					client->points--;
					sendResponse(client->fifoID, buildResult(answer, client, result));
				}
				else //Altro
				{
					sendResponse(client->fifoID, buildResult(answer, client, result));
				}
				
				//Libero le risorse	
				free(answer);
			}
		}
		return 0;
	}
}

