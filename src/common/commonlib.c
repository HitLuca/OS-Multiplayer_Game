//Autori
//Simonetto Luca 166540
//Federici Marco 165183
//Progetto Sistemi Operativi 1 2015
//Multiplayer Game

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>

#include "commonlib.h"



//Funzione di parsing dei messaggi Server->Client e Client->Server
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

//Funzione di parsing di messaggi in genere, per separare i vari messaggi se nella FIFO ne sono arrivati più di uno e devono essere ancora letti
Message** parseMessages(char *rawMessages,int size){
	//Conto il numero di separatori
	int separator=1;
	int i;
	char* start=rawMessages;
	for(i=0;i<size;i++)
	{
		if(rawMessages[i]=='\0')
		{
			separator++;
		}
	}

	Message **messages =(Message**)malloc(sizeof(Message*)*separator);
	char* position=rawMessages;
	int j=0;
	for(i=0;i<size;)
	{
		position+=i;
		i+=(int)strlen(position);
		i++;
		messages[j]=parseMessage(position);
		j++;
	}
	messages[j]=NULL;
	return messages;
}

//Permette di scrivere con vari colori e tag in modo da rendere più immediato l'output del terminale
void printScreen(int color, tags tag, char* message)
{
	if (color==1) //Voglio i colori
	{
		switch(tag)
		{
			case GAME:
			{
				printf("[" COLOR_GREEN "GAME" COLOR_RESET "] %s", message);
				break;
			}
			case INFO:
			{
				printf("[" COLOR_YELLOW "INFO"  COLOR_RESET "] %s", message);
				break;
			}
			case ERROR:
			{
				printf("[" COLOR_RED "ERROR"  COLOR_RESET "] %s", message);
				break;
			}
			case AUTH:
			{
				printf("[" COLOR_CYAN "AUTH"  COLOR_RESET "] %s", message);
				break;
			}
			case DEFAULT:
			{
				printf("%s", message);
				break;
			}
		}
	}
	else //Bianco e nero
	{
		switch(tag)
		{
			case GAME:
			{
				printf("[GAME] %s", message);
				break;
			}
			case INFO:
			{
				printf("[INFO] %s", message);
				break;
			}
			case ERROR:
			{
				printf("[ERROR] %s", message);
				break;
			}
			case AUTH:
			{
				printf("[AUTH] %s", message);
				break;
			}
			case DEFAULT:
			{
				printf("%s", message);
				break;
			}
		}
	}
	printf("\n");
}

//Print su file
void printFile(FILE* file, tags tag, char* message)
{
	switch(tag)
	{
		case GAME:
		{
			fprintf(file,"[GAME] %s", message);
			break;
		}
		case INFO:
		{
			fprintf(file,"[INFO] %s", message);
			break;
		}
		case ERROR:
		{
			fprintf(file,"[ERROR] %s", message);
			break;
		}
		case AUTH:
		{
			fprintf(file,"[AUTH] %s", message);
			break;
		}
		case DEFAULT:
		{
			fprintf(file,"%s", message);
			break;
		}
	}
	fprintf(file,"\n");
}

void printTitle(int color)
{
	if(color!=0)
	printf(COLOR_GREEN);
	printf(" __  __       _ _   _       _                       \n");
	printf("|  \\/  |     | | | (_)     | |                      \n");
	printf("| \\  / |_   _| | |_ _ _ __ | | __ _ _   _  ___ _ __ \n");
	printf("| |\\/| | | | | | __| | '_ \\| |/ _` | | | |/ _ | '__|\n");
	printf("| |  | | |_| | | |_| | |_) | | (_| | |_| |  __| |   \n");
	printf("|_|  |_|\\__,_|_|\\__|_| .__/|_|\\__,_|\\__, |\\___|_|   \n");
	if(color!=0)
	printf(COLOR_RED);
	printf("             / ____| ");
    if(color!=0)
	printf(COLOR_GREEN);
    printf("| |");
    printf("             ");
    printf("__/ |");
    if(color!=0)
    printf(COLOR_RED);
    printf("          \n");
	printf("            | |  __  ");
	if(color!=0)
	printf(COLOR_GREEN);
	printf("|_|");
	if(color!=0)
	printf(COLOR_RED);
	printf("_ _ __ ___  ");
	if(color!=0)
	printf(COLOR_GREEN);
	printf("|___/");
	if(color!=0)
	printf(COLOR_RED);
	printf("           \n");
	printf("            | | |_ |/ _` | '_ ` _ \\ / _ \\           \n");
	printf("            | |__| | (_| | | | | | |  __/           \n");
	printf("             \\_____|\\__,_|_| |_| |_|\\___|           \n");
	printf("                                                    \n");      
	if(color!=0)         
	printf(COLOR_RESET);                               
}
