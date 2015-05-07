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
		//Creo il thread con la parte di autorizzazione
		pthread_t authorization;
		pthread_create (&authorization, NULL, &authorizationThread, NULL);

		
		connectedClientsNumber=0;
		clientsMaxNumber=10; //TODO imposta il massimo da parametro
		initializeClientData();
		strcpy(currentQuestion.id, "0");
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

