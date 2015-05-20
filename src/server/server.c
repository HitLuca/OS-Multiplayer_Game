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

//Array di argomenti da passare al sender (nomeFIFOclient e messaggio)
//METTERE A POSTO WTF

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
		printScreen(colorRun, ERROR, "Server gia presente\n");
		return 0;
	}
	else
	{
		//Setto le variabili iniziali
		connectedClientsNumber=0;
		currentQuestion=0;
		
		//TODO settare con il valore passato
		
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

		if(testRun==1)
		{
			char filePath[1000];
			strcpy(filePath,"../assets/server/questions.test");
			testFile = fopen(filePath,"r");
			if(testFile==NULL)
			{
				printScreen(colorRun, ERROR, "Errore apertura file di test\n\n");
				return 0;
			}
		}

		printScreen(colorRun, DEFAULT, "\e[1;1H\e[2J");
		printScreen(colorRun, DEFAULT, "Benvenuto nel terminale utente!\n");
		printScreen(colorRun, DEFAULT, "Digita help per una lista dei comandi\n");

		//Creo il thread con la parte di autorizzazione
		pthread_t authorization;
		pthread_create (&authorization, NULL, &authorizationThread, NULL);


		//Inizializzazione delle strutture con i dati dei client e domande
		connectedClientsNumber=0;
		initializeClientData();

		currentQuestion=0;
		InitializeQuestions();
		GenerateNewQuestion();


		//Creo il thread per i comandi utente
		pthread_t bash;
		pthread_create (&bash, NULL, &bashThread, NULL);

		mkfifo(SERVER_ANSWER_FIFO,FILE_MODE);
		serverAnswerFIFO = open(SERVER_ANSWER_FIFO,O_RDWR);

		//Il server legge le risposte da serverAnswerFIFO
		char message[MAX_MESSAGE_SIZE*clientsMaxNumber];

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

				sprintf(stringBuffer, "%s ha risposto %s alla domanda %s\n", clientData[atoi(answer->parameters[0])]->name, answer->parameters[2],  questions[atoi(answer->parameters[1])].question->text);
				printScreen(colorRun, GAME, stringBuffer);
				
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

