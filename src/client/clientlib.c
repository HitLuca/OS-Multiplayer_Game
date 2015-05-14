#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>

#include "clientlib.h"

void* userInput(void* arg)  //METTERE A POSTO QUESTO THREAD, NON VA BENE
{
	//Il client invia le risposte ad serverAnswerFIFO
	char input[MAX_MESSAGE_SIZE];
	while(1)
	{
		printf("La domanda e' %s\n", currentQuestion.text);
		printf("La tua risposta>");
		waitingForUserInput=1;
		scanf("%s",input);
		fflush(stdin);
		sendResponse(serverAnswerFIFO, input);
		waitingForUserInput=0;
		//mi metto in attesa di una risposta
		pthread_mutex_lock(&mutex);
	}
}

int validateUsername(char* username)
{
	printf("hai scelto %s\n",username);
	if(strlen(username)<MIN_USERNAME_LENGHT)
	{
		printf("Errore: username troppo corto\n");
		return -1;
	}
	else if(strlen(username)>MAX_USERNAME_LENGHT)
	{
		printf("Errore: username troppo lungo\n");
		return -1;
	}
	else
	{
		int i;
		for(i=0;i<strlen(username);i++)
		{
			if(!isalnum(username[i]))
			{
				printf("Errore: carattere %c non valido\n",username[i]);
				return -1;
			}
		}
		return 0;
	}
	
}

char* authRequestMessage(char* pid,char* name)
{
	char *message = (char*)malloc(sizeof(char)*(strlen(pid)+strlen(name)+2));
	strcpy(message,pid);
	strcat(message,"|");
	strcat(message,name);
	return message;
}

int checkServerAuthResponse(Message* message){
	if(strcmp(message->parameters[0],"A")==0)
	{
		if(message->parameterCount==2)
		{
			return atoi(message->parameters[1]);
		}
		else if(message->parameterCount==5)
		{
			return 0;
		}
	}
	return -1;
}

void initializeClientData(Message *message){
	clientData->id=(char*)malloc(sizeof(char)*(strlen(message->parameters[1])+1));
	strcpy(clientData->id,message->parameters[1]);
	clientData->points=(char*)malloc(sizeof(char)*(strlen(message->parameters[4])+1));
	strcpy(clientData->points,message->parameters[4]);
}

void sendResponse(int serverAnswerFIFO, char* answer)
{
	char* message= malloc(MAX_MESSAGE_SIZE*sizeof(char)); //<----- MALLOC FATTA A CASO, SETTARE I VALORI CORRETTI!
	strcpy(message, clientData->id);
	strcat(message, "|");
	strcat(message, currentQuestion.id);
	strcat(message, "|");
	strcat(message, answer);
	printf("SendResponse ha creato: %s\n", message);
	write(serverAnswerFIFO,message,strlen(message)+1);
}

void initializeQuestion(Message *message)
{
	strcpy(currentQuestion.text,message->parameters[2]);
	strcpy(currentQuestion.id,message->parameters[3]);
}


void setNewQuestion(Message *message)
{
	strcpy(currentQuestion.text,message->parameters[2]);
	strcpy(currentQuestion.id,message->parameters[1]);
}

int DisplayResult(Message* message)
{
	int result=-1;
	if(message->parameters[3][0]=='C')
	{
		printf("-Risposta Corretta!\n");
		result = 1;
	}
	else if(message->parameters[3][0]=='W')
	{
		printf("-Risposta Sbagliata!\n");
		result = 0;
	}
	else if(message->parameters[3][0]=='T')
	{
		printf("-La risposta e' corretta ma qualcuno ha risposto prima di te!\n");
		result = 2;
	}
	
	printf("-Ora hai %s punti\n",message->parameters[2]);
	return result;
}
