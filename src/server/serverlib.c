#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>

#include "serverlib.h"

void* authorizationThread(void* arg)
{
	//Avvio il monitoraggio delle autorizzazioni
	int serverAuthFIFO = open(SERVER_AUTHORIZATION_FIFO,O_RDWR);
	char clientMessage[MAX_MESSAGE_SIZE];
	char serverMessage[MAX_MESSAGE_SIZE];
	char fifoPath [MAX_FIFO_NAME_SIZE];
	
	char* answer;
	
	while(1)
	{
		strcpy(fifoPath,CLIENT_MESSAGE_FIFO);
		int size = read(serverAuthFIFO,clientMessage,MAX_MESSAGE_SIZE);
		printf("authThread: Ho ricevuto %s \n",clientMessage);
		Message **messages=parseMessages(clientMessage,size);
		int i=0;
		while(messages[i]!=NULL)
		{
			Message *message = messages[i];
			i++;
			
			strcat(fifoPath,message->parameters[0]);
			
			printf(" %s ha effettuato una richiesta di connessione\n",message->parameters[1]);
			
			//apro la fifo del client
			int clientMessageFIFO = open(fifoPath,O_RDWR);
			
			int id=checkClientAuthRequest(message);
			
			if(id>=0)
			{
				connectNewClient(id,message->parameters[1],clientMessageFIFO);
				answer=authAcceptMessage(id);
			}
			else
			{
				answer=authRejectMessage(id);
			}
			write(clientMessageFIFO,answer, strlen(answer)+1);
			free(message);
			free(answer);
		}
		free(messages);
	}
}

void* bashThread(void*arg)
{
	char comando[MAX_COMMAND_SIZE];
	printf("\e[1;1H\e[2J");
	printf("Benvenuto nel terminale utente!\nDigita help per una lista dei comandi\n");
	while(1)
	{
		printf(">");
		scanf("%s", comando);
		if (strcmp(comando, "help")==0)
		{
			printf("Lista comandi utente:\n");
			printf("help: Richiama questo messaggio di aiuto\n");
			printf("kick <utente>: Kick di <utente> dalla partita\n");
			printf("question <question>: Invia una nuova domanda a tutti gli utenti\n");
			printf("list: Stampa la lista degli utenti connessi\n");
			printf("clear: Pulisce la schermata corrente\n");
		}
		else if (strcmp(comando, "clear")==0)
		{
			printf("\e[1;1H\e[2J");
		}
		else
		{
			printf("%s:Comando non riconosciuto\n",comando);
		}
	}
}

int checkClientAuthRequest(Message *message)
{
	if(message->parameterCount!=2)
	{
		return -2; //WRONG PARAMETER COUNT
	}
	else
	{
		if (connectedClientsNumber<clientsMaxNumber)
		{
			int i;
			for(i=0;i<clientsMaxNumber;i++)
			{
				if(clientData[i]!=NULL)
				{
					if(strcmp(clientData[i]->name,message->parameters[1])==0)
					{
						return -4; //NAME ALREADY IN USE
					}
				}
			}
			for(i=0;i<clientsMaxNumber;i++)
			{
				if(clientData[i]==NULL)
				{
					/*clientNames[i]=(char*)malloc((srtlen(message->parameters[1])+1)*sizeof(char));
					strcpy(clientNames[i],message->parameters[1]);
					connectedClientsNumber++;*/
					return i;
				}
			}
			
		}
		else
		{
			return -3; //SERVER FULL
		}
	}
}

void initializeClientData()
{
	clientData = (ClientData**)malloc(sizeof(ClientData*)*clientsMaxNumber);
	int i;
	for(i=0;i<clientsMaxNumber;i++)
	{
		clientData[i]=NULL;
	}
}

void connectNewClient(int id,char* name,int fifoID)
{
	clientData[id]=(ClientData*)malloc(sizeof(ClientData));
	clientData[id]->name=(char*)malloc(sizeof(char)*(strlen(name)+1));
	strcpy(clientData[id]->name,name);
	clientData[id]->fifoID=fifoID;
	clientData[id]->points=clientsMaxNumber-connectedClientsNumber;
	connectedClientsNumber++;
	printf("Ho allocato lo slot %d con il client %s che ha fifoID %d e parte con %d punti, sono rimasti %d posti liberi\n",id,clientData[id]->name,clientData[id]->fifoID,clientData[id]->points,clientsMaxNumber-connectedClientsNumber);
}

char* authAcceptMessage(int id)
{
	char idc[3];
	char points[10];
	sprintf(idc,"%d",id);
	sprintf(points,"%d",clientData[id]->points);
	char *answer = (char*)malloc(sizeof(char)*MAX_MESSAGE_SIZE);
	strcpy(answer,"A|");
	strcat(answer,idc);
	strcat(answer,"|");
	strcat(answer,questions[currentQuestion].question->text);
	strcat(answer,"|");
	strcat(answer,questions[currentQuestion].question->id);
	strcat(answer,"|");
	strcat(answer,points);
	return answer;
}

char* authRejectMessage(int error)
{
	char errors[4];
	sprintf(errors,"%d",error);
	char *answer = (char*)malloc(sizeof(char)*MAX_MESSAGE_SIZE);
	strcpy(answer,"A|");
	strcat(answer,errors);
	return answer;
}

int checkAnswer(Message* message)
{
	int questionIndex = atoi(message->parameters[1]);
	if (strcmp(questions[questionIndex].answer, message->parameters[2])==0)
	{
		if (questionIndex==currentQuestion)
		{
			return 1; //Risposta giusta domanda corrente
		}
		else
		{
			return 3; //Risposta giusta domanda vecchia
		}
	}
	else
	{
		return 2; //Risposta sbagliata
	}
}

char* buildResult(Message* message, ClientData* player, int cwt)
{
	char castedPoints[POINT_SIZE];
	sprintf(castedPoints, "%d", player->points);
	char* idQuestion = message->parameters[1];
	char* answer = malloc((6+strlen(idQuestion)+strlen(castedPoints))*sizeof(char));
	strcpy(answer, "R|");
	strcat(answer, idQuestion);
	strcat(answer, "|");
	strcat(answer, castedPoints);
	strcat(answer, "|");
	if (cwt==1)
	{
		strcat(answer, "C");
	}
	else if(cwt==2)
	{
		strcat(answer, "W");
	}
	else
	{
		strcat(answer, "T");
	}
	return answer;
}

ClientData* getSender(Message* message)
{
	int id = atoi(message->parameters[0]);
	return clientData[id];
}

void sendResponse(int fifoID, char* response)
{
	write(fifoID,response, strlen(response)+1);
	free(response);
}

void InitializeQuestions()
{
	currentQuestion=QUESTION_ID;
	srand(time(NULL));
}

void GenerateNewQuestion(){
	currentQuestion=(currentQuestion+1)%QUESTION_ID;
	
	if(questions[currentQuestion].question!=NULL)
	{
		free(questions[currentQuestion].question);
	}
	if(questions[currentQuestion].answer!=NULL)
	{
		free(questions[currentQuestion].answer);
	}
	
	char nums1[10];
	char nums2[10];
	char *answs=(char*)malloc(11*sizeof(char));
	int num1,num2,answ;
	char op[2];
	
	num1=rand()%MAX_QUESTION_NUM;
	num2=rand()%MAX_QUESTION_NUM;
	
	//TODO OPTION WITH OTHER OPERATIONS
	op[0]='+';
	op[1]='\0';
	
	sprintf(nums1,"%d",num1);
	sprintf(nums2,"%d",num2);
	
	char newText[MAX_QUESTION_SIZE];
	strcpy(newText,nums1);
	strcat(newText," ");
	strcat(newText,op);
	strcat(newText," ");
	strcat(newText,nums2);
	strcat(newText," = ?");
	
	char newId[MAX_QID_SIZE];
	sprintf(newId,"%d",currentQuestion);
	
	if(strcmp(op,"+")==0) //TODO add other operations
	{
		answ=num1+num2;
	}
	
	sprintf(answs,"%d",answ);
	
	Question* newQuestion = (Question*) malloc(sizeof(Question));
	strcpy(newQuestion->id,newId);
	strcpy(newQuestion->text,newText);
	
	questions[currentQuestion].question=newQuestion;
	questions[currentQuestion].answer=answs;
	printf("Ho generato la domanda %s\n",newText);
}

void BroadcastQuestion(){
	int i;
	char newQuestionMessage [MAX_MESSAGE_SIZE];
	strcpy(newQuestionMessage,"Q|");
	strcat(newQuestionMessage,questions[currentQuestion].question->id);
	strcat(newQuestionMessage,"|");
	strcat(newQuestionMessage,questions[currentQuestion].question->text);
	for(i=0;i<clientsMaxNumber;i++){
		if(clientData[i]!=NULL)
		{
			write(clientData[i]->fifoID,newQuestionMessage,strlen(newQuestionMessage)+1);
		}	
	}
}


