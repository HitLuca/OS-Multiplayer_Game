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
		//genero il nome della mia fifo univocamente utilizzando il PID e faccio il paring
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
				serverAnswerFIFO = open(SERVER_ANSWER_FIFO,O_RDWR);
				
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
				initializeQuestion(answer);
				
				printf("Il server mi ha assegnato l'id %s e %s punti\n",clientData->id,clientData->points);

				//Componenti del thread bash
				pthread_t bash;
				char arg[MAX_MESSAGE_SIZE];
				
				pthread_create (&bash, NULL, &userInput, arg);
				
				char rawMessages[MAX_MESSAGE_SIZE];
				
				//setto il mutex a 0
				pthread_mutex_lock(&mutex);
				waitingForUserInput=0;
				
				while (1)
				{
					//mi metto in ascolto
					int size = read(inMessageFIFO,rawMessages,MAX_MESSAGE_SIZE*MAX_CONCURRENT_MESSAGES);
					if(size>0)
					{
						printf("Ho ricevuto: %s  di dimensione %d\n",rawMessages,size);
						
						Message** messageList = parseMessages(rawMessages,size); 
						int i=0;
						
						while(messageList[i]!=NULL)
						{
							Message* message = messageList[i];
							i++;
							//se ricevo il messaggio di kick mi chiudo
							if(message->parameters[0][0]=='K')
							{
								printf("Kicked by server\n");
								return 0;
							}
							else 
							{
								//controllo se mi ha inviato una nuova domanda o una risultato
								if(strchr(message->parameters[0],'R')!=NULL) //esito di una risposta inviata
								{
									if(DisplayResult(message)==0)
									{
										//sblocco il thread di bash
										pthread_mutex_unlock(&mutex);
									}
								}
								else if(strchr(message->parameters[0],'Q')!=NULL) //nuova domanda
								{
									setNewQuestion(message);
									if(waitingForUserInput==1)
									{
										waitingForUserInput=0;
										if (pthread_cancel(bash)!=0)
										{
											printf("Impossibile terminare il thread bash :(\n");
										}
										//creo un nuovo thread a cui associo l'input dell'utente
										pthread_create (&bash, NULL, &userInput, &arg);
									}
									else
									{
										pthread_mutex_unlock(&mutex);
									}
								}
								else
								{
									printf("Messaggio sconosciuto ricevuto: %s \n",rawMessages);
									return 0;
								}
							}
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
