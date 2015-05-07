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