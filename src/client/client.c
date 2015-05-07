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

#include "clientlib.h"

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
		//genero il nome della mia fifo univocamente utilizzando il PID e la paro
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
