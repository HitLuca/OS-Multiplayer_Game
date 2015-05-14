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
