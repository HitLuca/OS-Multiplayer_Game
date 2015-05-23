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

#include "clientlib.h"

//Handler per le uscite dal programma, libero le risorse e avverto che mi sono disconnesso
//{Q|idClient}
void handler ()
{  
	print(DEFAULT, "");
	if(connected==1)
	{
		char disconnectMessage[MAX_MESSAGE_SIZE];
		strcpy(disconnectMessage,"Q|");
		strcat(disconnectMessage,clientData->id);
		write(serverAuthFIFO,disconnectMessage,strlen(disconnectMessage)+1);
	}
	deallocResources();
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
			sprintf(stringBuffer, "La domanda e' %s", currentQuestion.text);
			print( GAME, stringBuffer);
			newQuestion=0;
		}
		printf("La tua risposta>");
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

//Funzione di lettura dell'input associato ai client in modo da riconoscere risposte, ritardi ecc.
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
	sprintf(stringBuffer, "Hai scelto %s",username);
	print( INFO, stringBuffer);
	if(strlen(username)<MIN_USERNAME_LENGHT)
	{
		print( ERROR, "Username troppo corto");
		return -1;
	}
	else if(strlen(username)>MAX_USERNAME_LENGHT)
	{
		print( ERROR, "Username troppo lungo");
		return -1;
	}
	else
	{
		int i;
		for(i=0;i<strlen(username);i++)
		{
			if(!isalnum(username[i]))
			{
				sprintf(stringBuffer, "Carattere %c non valido",username[i]);
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
	if (inMessageFIFO!=NULL)
	{
		close(inMessageFIFO);
		unlink(messageFIFOName);
	}
	if(testFile!=NULL)
	{
		fclose(testFile);
	}
	if(logFile!=NULL)
	{
		fclose(logFile);
	}
}

void printRanking(char** ranking,int size)
{
	int i,j;
	print(GAME,"La partita si e' conclusa");
	if(strcmp(ranking[0],username)==0)
	{
		print(GAME,"Hai vinto!");
	}
	else
	{
		print(GAME,"Hai perso!");
	}
	fprintf(logFile,"\n\tGIOCATORE\t\t\tPUNTI\n");
	fprintf(logFile,"    -------------------------------------------\n");
	for(i=0;i<size;i+=2)
	{
		fprintf(logFile,"\t%s",ranking[i]);
		for(j=0;j<=4*8;j+=8)
		{
			if(strlen(ranking[i])<j)
			{
				fprintf(logFile,"\t");
			}
		}
		fprintf(logFile,"%s\n",ranking[i+1]);
	}
	fprintf(logFile,"\n");
	if(testRun==0)
	{
		int max=atoi(ranking[1]);
		int clientNumber=size/2;
		int terminalSize=40;
		int p;
		int d;
		char block1[]="\u2588";
		char block2[]="\u2592";
		char block3[]="\u2591";

	
		useconds_t useconds=10000L;
		printf("\n\n");
		for(i=0;i<size;i+=2)
		{
			if(colorRun!=0)
			{
				if((float)(i+1)/2<=(float)clientNumber/3.0)
				{
					printf(COLOR_GREEN);
				}
				else if((float)(i+1)/2<=(float)clientNumber*2.0/3.0)
				{
					printf(COLOR_YELLOW);
				}
				else
				{
					printf(COLOR_RED);
				}
			}
			
			printf("\t%s",ranking[i]);
			
			p=atoi(ranking[i+1]);
			d=(int)(((float)p/(float)max)*(float)terminalSize);
			for(j=0;j<=2*8;j+=8)
			{
				if(strlen(ranking[i])<j)
				{
					printf("\t");
				}
				
			}
			for(j=0;j<d;j++)
			{
				printf("%s",block1);
				usleep(useconds);
				fflush(stdout);
			}
			if(colorRun)
			{
				printf(COLOR_RESET);
			}
			printf("\n");
		}
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


