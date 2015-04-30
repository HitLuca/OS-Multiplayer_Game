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

#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

int serverAuthFIFO;

void encapsuleMessage(char *message);
void parseMessage(char *rawMessage);
void* userInput(void* arg);

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
		
		//mando un messaggio al server richiedendo l'autorizzazione e passandogli il mio pid
		char message[MAX_MESSAGE_SIZE];
		strcpy(message,pid);
		encapsuleMessage(message);
		if(write(serverAuthFIFO,message,strlen(message)))
		{
			//aspetto una risposta
			char answer[MAX_MESSAGE_SIZE];
			read(inMessageFIFO,answer,MAX_MESSAGE_SIZE);
			parseMessage(answer);
			
			if(strcmp(answer, "0")==0)	//se la risposta è negativa significa che il server è pieno
			{
				printf("Server Pieno\n");
				return 0;
			}
			else if(strcmp(answer, "1")==0)	//altrimenti setto i parametri e proseguo
			{
				printf("Connessione Riuscita\n");

				//provo ad aprire la FIFO delle risposte alle domande da inviare al server
				int serverAnswerFIFO = open(SERVER_ANSWER_FIFO,O_RDWR);
				
				if(serverAnswerFIFO == -1)
				{
					printf("Errore di connessione al server\n");
					return 0;
				}

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
						parseMessage(message);
						//se ricevo il messaggio di kick mi chiudo
						if(strcmp(message, "k")==0)
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

void encapsuleMessage(char *message)
{
	message[strlen(message)]='!';
}

void parseMessage(char *rawMessage)
{
	*strchr(rawMessage,'!')='\0';
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
		encapsuleMessage(input);
		write(serverAnswerFIFO,input,strlen(input));
	}
}