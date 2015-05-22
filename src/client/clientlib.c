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

//Handler per le uscite dal programma, libero le risorse e avverto che mi sono disconnesso
//{Q|idClient}
void handler ()
{  
	deallocResources();
	print(DEFAULT, "\n");
	if(connected==1)
	{
		char disconnectMessage[MAX_MESSAGE_SIZE];
		strcpy(disconnectMessage,"Q|");
		strcat(disconnectMessage,clientData->id);
		write(serverAuthFIFO,disconnectMessage,strlen(disconnectMessage)+1);
	}
	exit(0);
}

//Thread bash che legge le risposte client e notifica delle nuove domande
void* userInput(void* arg)  
{
	//Il client invia le risposte ad serverAnswerFIFO
	char* input=(char*)malloc(sizeof(char)*MAX_MESSAGE_SIZE);
	size_t size = MAX_MESSAGE_SIZE;
	
	while(1)
	{
		if(newQuestion==1)
		{
			sprintf(stringBuffer, "La domanda e' %s\n", currentQuestion.text);
			print( GAME, stringBuffer);
			newQuestion=0;
		}
		print( DEFAULT, "La tua risposta>");
		waitingForUserInput=1;
		getline(&input,&size,stdin);
		strchr(input,'\n')[0]='\0';
		fflush(stdin);
		sendResponse(serverAnswerFIFO, input);
		waitingForUserInput=0;
		//mi metto in attesa di una risposta
		pthread_mutex_lock(&mutex);
	}
}

void* testInput(void* arg)  
{
	//Il client invia le risposte ad serverAnswerFIFO
	char* input=(char*)malloc(sizeof(char)*MAX_MESSAGE_SIZE);
	
	char* answer=(char*)malloc(sizeof(char)*20);
	char* delay=(char*)malloc(sizeof(char)*20);
	while(1)
	{
		fscanf(testFile,"%s",delay);
		if(strcmp(delay,"end")==0)
		{
			break;
		}
		fscanf(testFile,"%s",answer);
		waitingForUserInput=1;
		usleep(atoi(delay)*1000L);
		sendResponse(serverAnswerFIFO, answer);
		waitingForUserInput=0;
		pthread_mutex_lock(&mutex);
	}
}

//Funzione di validazione del nome scelto dal giocatore
int validateUsername(char* username)
{
	sprintf(stringBuffer, "Hai scelto %s\n",username);
	print( INFO, stringBuffer);
	if(strlen(username)<MIN_USERNAME_LENGHT)
	{
		print( ERROR, "Username troppo corto\n");
		return -1;
	}
	else if(strlen(username)>MAX_USERNAME_LENGHT)
	{
		print( ERROR, "Username troppo lungo\n");
		return -1;
	}
	else
	{
		int i;
		for(i=0;i<strlen(username);i++)
		{
			if(!isalnum(username[i]))
			{
				sprintf(stringBuffer, "Carattere %c non valido\n",username[i]);
				print( DEFAULT, stringBuffer);
				return -1;
			}
		}
		return 0;
	}
	
}

//Funzione di invio del messaggio di autentificazione
char* authRequestMessage(char* pid,char* name)
{
	char *message = (char*)malloc(sizeof(char)*(strlen(pid)+strlen(name)+4));
	strcpy(message,"R|");
	strcat(message,pid);
	strcat(message,"|");
	strcat(message,name);
	return message;
}

//Check della risposta del server in seguito alla mia richiesta di autentificazione
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

//Inizializzazione dei dati client
void initializeClientData(Message *message){
	clientData->id=(char*)malloc(sizeof(char)*(strlen(message->parameters[1])+1));
	strcpy(clientData->id,message->parameters[1]);
	clientData->points=(char*)malloc(sizeof(char)*(strlen(message->parameters[4])+1));
	strcpy(clientData->points,message->parameters[4]);
}

//Routine di invio della risposta al server nella serverAnswerFIFO
//{idClient|idQuestion|answer}
void sendResponse(int serverAnswerFIFO, char* answer)
{
	if(logFile!=NULL)
	{
		fprintf(logFile,answer);
		fprintf(logFile,"\n");
	}
	char* message= malloc(MAX_MESSAGE_SIZE*sizeof(char)); 
	strcpy(message, clientData->id);
	strcat(message, "|");
	strcat(message, currentQuestion.id);
	strcat(message, "|");
	strcat(message, answer);
	write(serverAnswerFIFO,message,strlen(message)+1);
}

//Inizializzazione della domanda da messaggio passato in argomento
void initializeQuestion(Message *message)
{
	strcpy(currentQuestion.text,message->parameters[2]);
	strcpy(currentQuestion.id,message->parameters[3]);
}

//Set della nuova domanda
void setNewQuestion(Message *message)
{
	newQuestion=1;
	strcpy(currentQuestion.text,message->parameters[2]);
	strcpy(currentQuestion.id,message->parameters[1]);
}

//Deallocazione delle risorse con unlink delle FIFO client
void deallocResources(){
	close(inMessageFIFO);
	unlink(messageFIFOName);
	if(testFile!=NULL)
	{
		fclose(testFile);
	}
	if(logFile!=NULL)
	{
		fclose(logFile);
	}
}

void print(tags tag,char* message)
{
	if(testRun==0)
	{
		printf("\r                                \r");
		printScreen(colorRun,tag,message);
	}
	if(logFile!=NULL)
	{	
		printFile(logFile,tag,message);
		fflush(logFile);
	}
}
