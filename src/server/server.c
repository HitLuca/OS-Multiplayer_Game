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

int main(int argc,char **argv)
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
		maxPoints=11; //TODO imposta da parametro
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
		//printf("answThread: In lettura:\n");
		while (1) 
		{
			int size=read(serverAnswerFIFO,message,MAX_MESSAGE_SIZE*clientsMaxNumber);
			printf("answThread: Ho ricevuto %s nella FIFO risposte\n", message);
			Message **messages=parseMessages(message,size);
			int i=0;
			while(messages[i]!=NULL)
			{
				Message* answer = messages[i];
				i++;
				printf("Il client con ID %s ha risposto %s alla domanda con ID %s\n", answer->parameters[0], answer->parameters[2],  answer->parameters[1]);
				
				int result = checkAnswer(answer);
				printf("result: %d\n", result);
				
				ClientData* client = getSender(answer);

				if (result==1)
				{
					client->points++;
					sendResponse(client->fifoID, buildResult(answer, client, result));
					if(client->points>=maxPoints)
					{
						endGame(client);
					}
					else
					{
						//Nuova domanda
						GenerateNewQuestion();
						BroadcastQuestion();
					}
				}
				else if (result==2)
				{
					client->points--;
					sendResponse(client->fifoID, buildResult(answer, client, result));
				}
				else
				{
					sendResponse(client->fifoID, buildResult(answer, client, result));
				}

				
				free(answer); //<---------------------7
			}
		}
		return 0;
	}
}

