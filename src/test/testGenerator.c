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

#define MAX_QUESTION_SIZE 1000
#define MAX_QUESTION_NUM 100
#define MAX_WAIT 100
#define CORRECT_PERCENTAGE 75
#define MIN_OFFSET 0

void GenerateNewQuestion(int index);

FILE* questionsFile;
FILE** playerFile;
char** questions;
char** answers;
int clientsNumber;
int maxPoints;

int main(int argc,char** argv)
{
	char questionsFilePath[1000];
	char clientFilePath[1000];
	char logFilePath[1000];
	FILE* logFile;
	int questionNumber=100;
	if(argc<2 || argc >3)
	{
		fprintf(stderr,"Il programma richiede 1 o 2 argomenti: <nClients> <maxPoints>\n");
		return 0;
	}
	
	clientsNumber=atoi(argv[1]);
	if (clientsNumber<0)
	{
		fprintf(stderr,"clientsNumber non può essere negativo\n");
		exit(EXIT_FAILURE);
	}
	if(argc==3)
	{
		maxPoints=atoi(argv[2]);
		if (maxPoints<0)
		{
			fprintf(stderr,"maxPoints non può essere negativo\n");
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		maxPoints=0;
	}
	
	long long timePassed=0;
	
	answers = (char**)malloc(sizeof(char*)*questionNumber);
	questions = (char**)malloc(sizeof(char*)*questionNumber);
	
	playerFile=(FILE**)malloc(sizeof(FILE*)*clientsNumber);
	
	if(maxPoints==0)
	{
		questionNumber=75;
	}
	
	//Path dei files di asset
	strcpy(questionsFilePath,"assets/server/questions.test");
	strcpy(logFilePath,"assets/server/server.log");
	strcpy(clientFilePath,"assets/client/");
	
	questionsFile = fopen(questionsFilePath,"w");

	if(questionsFile==NULL)
	{
		printf("Errore creazione file di test\n\n");
		exit(EXIT_FAILURE);
	}
	
	int i;
	for(i=0;i<questionNumber;i++)
	{
		GenerateNewQuestion(i);
	}
	fprintf(questionsFile,"end\n");
	fclose(questionsFile);
	
	
	//Inizializzo i dati
	int* points=(int*)malloc(sizeof(int)*clientsNumber);
	int* lastInteraction=(int*)malloc(sizeof(int)*clientsNumber);
	int* login=(int*)malloc(sizeof(int)*clientsNumber);
	for(i=0;i<clientsNumber;i++)
	{
		points[i]=-1;
		login[i]=0;
		lastInteraction[i]=0;
		char index[20];
		sprintf(index,"%d",i+1);
		char* path=(char*)malloc((strlen(clientFilePath)+20)*sizeof(char));
		strcpy(path,clientFilePath);
		strcat(path,index);
		strcat(path,".test");
		playerFile[i]=fopen(path,"w");
	}
	
	
	
	srand(time(NULL));
	int startPoints=clientsNumber;
	int currentQuestion=0;
	int delay,chosen;
	
	//Il log teorico viene generato solo per l'intensive test
	if(maxPoints>0)
	{
		logFile=fopen(logFilePath,"w");
		fprintf(logFile,"[GAME] La nuova domanda e' %s, ha risposta %s\n",questions[0],answers[0]);
	}
	int nAnswer=0;
	while(1)
	{
		chosen=rand()%clientsNumber;
		if(maxPoints==0)
		{
			delay=rand()%MAX_WAIT;
			delay+=MIN_OFFSET;
		}
		//nel caso di test intesivo è necessario un ampio ritardo per evitare inversioni dovute allo scheduler
		else
		{
			delay=1000;
		}
		
		timePassed+=delay;
		
		
		fprintf(playerFile[chosen],"%d\n",(int)(timePassed-lastInteraction[chosen]));
		lastInteraction[chosen]=timePassed;
		
		if(login[chosen]==0)
		{
			login[chosen]=1;
		
			fprintf(playerFile[chosen],"Player%d\n",chosen+1);
			points[chosen]=startPoints;
			if(maxPoints>0)
			{
				fprintf(logFile,"[AUTH] Player%d ha effettuato una richiesta di connessione\n",chosen+1);
				fprintf(logFile,"[AUTH] Ho accettato la richiesta di Player%d, e gli ho assegnato %d punti\n",chosen+1,startPoints);
				fprintf(logFile,"[INFO] Rimangono %d posti liberi\n",startPoints-1);
			}
			startPoints--;
		}
		else
		{
			//se sono in modalità test intesivo allora genero il file di log teorico
			if(maxPoints>0)
			{
				int correct=rand()%100;
				if(correct<CORRECT_PERCENTAGE)
				{
					fprintf(playerFile[chosen],"%s\n",answers[currentQuestion]);
					if(maxPoints>0)
					{
						fprintf(logFile,"[GAME] Player%d ha risposto %s alla domanda %s\n",chosen+1,answers[currentQuestion],questions[currentQuestion]);
						if(currentQuestion<questionNumber && points[chosen]<maxPoints-1)
						{
							fprintf(logFile,"[GAME] La nuova domanda e' %s, ha risposta %s\n",questions[currentQuestion+1],answers[currentQuestion+1]);
						}
					}
					points[chosen]++;
					currentQuestion++;
				}
				else
				{
					fprintf(playerFile[chosen],"error\n");
					if(maxPoints>0)
					{
						fprintf(logFile,"[GAME] Player%d ha risposto error alla domanda %s\n",chosen+1,questions[currentQuestion]);
					}
					points[chosen]--;
				}
			}
			else //altrimenti faccio rispondere i clients a caso
			{
				fprintf(playerFile[chosen],"%d\n",(rand()%2)+1);
				nAnswer++;
			}
			
			if(currentQuestion>questionNumber)
			{
				if(maxPoints>0)
				{
					fprintf(logFile,"[INFO] Sessione di test terminata\n");
				}
				break;
			}
			else if(points[chosen]>=maxPoints && maxPoints!=0)
			{
				fprintf(logFile,"[GAME] Il giocatore Player%d ha vinto\n",chosen+1);
				fprintf(logFile,"[GAME] La partita e' terminata\n\n");
				
				break;
			}
			else if(nAnswer==3*questionNumber*clientsNumber)
			{
				break;
			}
		}
		if(maxPoints>0)
		{
			fflush(logFile);
		}
		fflush(playerFile[chosen]);
	}
	if(maxPoints>0)
	{
		fclose(logFile);
	}
	for(i=0;i<clientsNumber;i++)
	{
		fprintf(playerFile[i],"end\n");
		fclose(playerFile[i]);
	}
	
	printf("[\x1b[33mINFO\x1b[0m]File di test generati,\n");
	if(maxPoints==0)
	{
		printf("[\x1b[33mINFO\x1b[0m]il test richiedera' al piu' %lld secondi\n",(timePassed)/1000);
	}
	else
	{
		printf("[\x1b[33mINFO\x1b[0m]il test richiedera' %lld secondi\n",(timePassed)/1000);
	}
	
	return 0;
}

//Funzione di generazione delle domande per il testing
void GenerateNewQuestion(int index){
	
	char *newText=(char*)malloc(MAX_QUESTION_SIZE*sizeof(char));
	char *answs=(char*)malloc(11*sizeof(char));


	char nums1[10];
	char nums2[10];
		
	int num1,num2,answ;
	char op[2];
	
	if(maxPoints>0) //Domanda casuale per make intensive_test
	{	
		num1=rand()%MAX_QUESTION_NUM;
		num2=rand()%MAX_QUESTION_NUM;
	}
	else //Domanda semplificata per make test (1+0 1+1)
	{	
		num1=rand()%2;
		num2=1;
	}
	
	op[0]='+';
	op[1]='\0';
	
	sprintf(nums1,"%d",num1);
	sprintf(nums2,"%d",num2);

	
	strcpy(newText,nums1);
	strcat(newText," ");
	strcat(newText,op);
	strcat(newText," ");
	strcat(newText,nums2);
	strcat(newText," = ?");
		
	answ=num1+num2;
	sprintf(answs,"%d",answ);	
	
	fprintf(questionsFile,"%s\n%s\n",newText,answs);
	fflush(questionsFile);
	answers[index]=answs;
	questions[index]=newText;
}
