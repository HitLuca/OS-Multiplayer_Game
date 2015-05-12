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
	//Check se il server è gia avviato
	if (mkfifo(SERVER_AUTHORIZATION_FIFO,FILE_MODE)!=0)
	{
		printf("Server gia presente\n");
		return 0;
	}
	else
	{
		//Setto le variabili incluse max e win
		connectedClientsNumber=0;
		currentQuestion=0;
		if (argv[1]!=0)
		{	
			clientsMaxNumber = atoi(argv[1]);
		}
		if (argv[2]!=0)
		{	
			winPoints = atoi(argv[2]);
		}

		//Creo il thread con la parte di autorizzazione
		pthread_t authorization;
		pthread_create (&authorization, NULL, &authorizationThread, NULL);

		initializeClientData();

		Question* question = (Question*)malloc(sizeof(Question));
		strcpy(question->id, "0");
		strcpy(question->text,"3 + 2 = ?");
		questions[currentQuestion].question=question;
		questions[currentQuestion].answer=(char*)malloc(sizeof(char)*5);
		strcpy(questions[currentQuestion].answer,"5");

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
			Message* answer = parseMessage(message);
			printf("Il client con ID %s ha risposto %s alla domanda con ID %s\n", answer->parameters[0], answer->parameters[2],  answer->parameters[1]);
			
			int result = checkAnswer(answer);
			printf("result: %d\n", result);
			
			ClientData* client = getSender(answer);

			if (result==1)
			{
				client->points++;

				//Nuova domanda
			}
			else if (result==2)
			{
				client->points--;
			}

			char* response=buildResult(answer, client, result);//!!!!!!
			sendResponse(client->fifoID, response);
			free(answer); //<---------------------
		}
		return 0;
	}
}

