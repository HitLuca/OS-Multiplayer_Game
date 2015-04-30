#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>

#define SERVER_ANSWER_FIFO "SANSF"
#define SERVER_AUTHORIZATION_FIFO "SAUTHF"
#define CLIENT_MESSAGE_FIFO "CMF"
#define MAX_MESSAGE_SIZE 1000

#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

int serverAuthFIFO;

void* userInput(void* arg){
	printf("Scrivi la tua risposta>");
	char input[10];
	scanf("%s",input);
	write(serverAuthFIFO,input,strlen(input));
}

int main()
{
	mkfifo(CLIENT_MESSAGE_FIFO,FILE_MODE);
	int inMessageFIFO = open(CLIENT_MESSAGE_FIFO,O_RDONLY | O_NONBLOCK);
	serverAuthFIFO = open(SERVER_AUTHORIZATION_FIFO,O_WRONLY | O_NONBLOCK);
	if(serverAuthFIFO==-1 )
	{
		printf("Server non presente\n");
		return 0;
	}
	else if(inMessageFIFO==-1)
	{
		printf("Errore di apertura FIFO\n");
		return 0;
	}
	else 
	{
		if(write(serverAuthFIFO,"tizio",sizeof("tizio")))
		{
			char *answer;
			read(inMessageFIFO,answer,MAX_MESSAGE_SIZE);
			if(answer[0]=='0')
			{
				printf("Server Pieno\n");
				return 0;
			} 
			else if(answer[0]=='1')
			{
				printf("Connessione Riuscita\n");
				int answerFIFO = open(SERVER_ANSWER_FIFO,O_WRONLY | O_NONBLOCK);
				
				while (1)
				{
					pthread_t bash;
					char *question;
					pthread_create (&bash, NULL, &userInput, &question);
					char *message;
					if(read(inMessageFIFO,message,MAX_MESSAGE_SIZE))
					{
						if(message[0]=='k')
						{
							printf("Kicked by server\n");
							return 0;
						}
					}
					else
					{
						printf("Errore in lettura messaggio server\n");
					}

				}
				return 0;
			}
		} else {
			printf("Errore richiesta autorizzazione\n");
		}
	}
	return 0;
}
