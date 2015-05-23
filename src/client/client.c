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

#include "clientlib.h"

int main(int argc, char** argv) //client --test --color
{
	struct sigaction sa;
	sa.sa_handler = &handler;
	  
	sigaction (SIGKILL, &sa, NULL);
	sigaction (SIGABRT, &sa, NULL);
	sigaction (SIGQUIT, &sa, NULL);
	sigaction (SIGINT, &sa, NULL);
	sigaction (SIGSEGV, &sa, NULL);
	
	connected=0;
	testRun=0;
	
	//Leggo i parametri
	if (strcmp(argv[1],"0")!=0)
	{	
		testRun = atoi(argv[1]);
		if (testRun<0)
		{
			fprintf(stderr, "testRun non può essere negativo\n");
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		testRun=0;
	}

	if (strcmp(argv[2],"0")!=0)
	{	
		colorRun = 1;
	}
	else
	{
		colorRun = 0;
	}
	
	//provo ad aprire la fifo di autorizzazione del server
	serverAuthFIFO = open(SERVER_AUTHORIZATION_FIFO,O_WRONLY);
	if(serverAuthFIFO==-1 )	//se la fifo non è presente significa che non vi è nessun server
	{
		print(ERROR, "Server non presente\n");
		return 1;
	}

	//genero il nome della mia fifo univocamente utilizzando il PID e faccio il paring
	char pid[100];
	sprintf(pid,"%d",getpid());
	strcpy(messageFIFOName, CLIENT_MESSAGE_FIFO);
	strcat(messageFIFOName,pid);
	
	mkfifo(messageFIFOName,FILE_MODE);
	inMessageFIFO = open(messageFIFOName,O_RDWR);

	//se l'apertura non va a buon fine stampo un errore
	if(inMessageFIFO==-1)
	{
		print( ERROR, "Errore di apertura FIFO\n");
		exit(EXIT_FAILURE);
	}
	
	
	//setto il percorso corrente
	char currentPath[500];
	char* c;
	char* past;
	strcpy(currentPath, argv[0]);
	c=currentPath;
	while(1)
	{
		past = c;
		c=strchr(c, '/');
		if (c==NULL)
		{
			break;
		}
		c++;
	}
	strcpy(past,"\0");
	
	char* testFileName=(char*)malloc(sizeof(char)*10);
	strcpy(testFileName,argv[1]);
	
	username=(char*)malloc(sizeof(char)*MAX_USERNAME_LENGHT);
	size_t size=MAX_USERNAME_LENGHT;
	
	if(testRun==0) //se non è una run di test chiedo di inserire un username
	{
		int correctUsername=-1;
		while(correctUsername==-1)
		{
			printf("Inserisci il tuo username> ");
			getline(&username,&size,stdin);
			strchr(username,'\n')[0]='\0';
			fflush(stdin);
			correctUsername = validateUsername(username);
		}
	}
	else //altrimenti lo leggo da file
	{
		char testFilePath[500];
		strcpy(testFilePath,currentPath);
		strcat(testFilePath,"../../assets/client/");
		strcat(testFilePath,testFileName);
		strcat(testFilePath,".test");
		testFile = fopen(testFilePath,"r");
		if(testFile==NULL)
		{
			print(ERROR, "Errore apertura file di test\n");
			exit(EXIT_FAILURE);
		}
		int size = 20;
		char delay[20];
		fscanf(testFile,"%s",delay);
		fscanf(testFile,"%s",username);
		usleep(atoi(delay)*1000l);
	}
	
	//apro il file di log
	char logFilePath[500];
	strcpy(logFilePath,currentPath);
	strcat(logFilePath,"../../log/client/");
	strcat(logFilePath,username);
	strcat(logFilePath,".log");
	logFile = fopen(logFilePath,"w");
	
	if(testRun==0)
	{
		printf("\e[1;1H\e[2J");
		printTitle(colorRun);
		if(colorRun)
		{
			printf("\t\t\t"COLOR_BLUE"CLIENT\n\m"COLOR_RESET);
		}
		else
		{
			printf("\t\t\tCLIENT\n\n");
		}
	}

	
	//mando un messaggio al server richiedendo l'autorizzazione e passandogli il mio pid e il mio username
	char *message = authRequestMessage(pid,username);
	
	
	if(write(serverAuthFIFO,message,strlen(message)+1))
	{
		//aspetto una risposta
		char answerBuffer[MAX_MESSAGE_SIZE];
		read(inMessageFIFO,answerBuffer,MAX_MESSAGE_SIZE);
		Message *answer = parseMessage(answerBuffer);
		
		
		int answerResult = checkServerAuthResponse(answer);
		
		if(answerResult<0) //Gestione degli errori di autentificazione
		{
			if (answerResult==-2)
			{
				print(ERROR, "Bad request\n");
			}
			else if(answerResult==-3)
			{
				print(ERROR, "Il server è pieno\n");
			}
			else if (answerResult==-4)
			{
				print(ERROR, "Il nome è già stato usato da un altro giocatore\n");
			}
			else if(answerResult==-5)
			{
				print(ERROR, "\"all\" non può essere scelto come username\n");
			}
			deallocResources();
			return answerResult;
			print( ERROR, stringBuffer);
			deallocResources();
			return answerResult;
		}
		else
		{
			//provo ad aprire la FIFO delle risposte alle domande da inviare al server
			serverAnswerFIFO = open(SERVER_ANSWER_FIFO,O_RDWR);
			
			if(serverAnswerFIFO == -1)
			{
				print( ERROR, "Errore di connessione al server\n");
				deallocResources();
				exit(EXIT_FAILURE);
			}
			
			print( INFO, "Connessione Riuscita");
			
			//inizializzo le variabili
			connected=1;
			clientData = (ClientData*) malloc(sizeof(ClientData));
			clientData->name=username;
			initializeClientData(answer);
			initializeQuestion(answer);
			
			sprintf(stringBuffer, "Il server mi ha assegnato %s punti",clientData->points);
			print( INFO, stringBuffer);

			//Componenti del thread bash
			pthread_t bash;
			
			
			char arg[MAX_MESSAGE_SIZE];
			if(testRun==0)
			{
				pthread_create (&bash, NULL, &userInput, arg);
			}
			else
			{
				pthread_create (&bash, NULL, &testInput, arg);
			}
			char rawMessages[MAX_MESSAGE_SIZE];
			
			//setto il mutex a 0
			pthread_mutex_lock(&mutex);
			waitingForUserInput=0;
			newQuestion=1;
			endGame=0;
			
			while (1)
			{
				//mi metto in ascolto
				int size = read(inMessageFIFO,rawMessages,MAX_MESSAGE_SIZE*MAX_CONCURRENT_MESSAGES);
				if(size>0)
				{
					Message** messageList = parseMessages(rawMessages,size); 
					int i=0;
					
					while(messageList[i]!=NULL)
					{
						Message* message = messageList[i];
						i++;
						
						if(strchr(message->parameters[0],'K')!=NULL) //messaggio di kick
						{
							print( AUTH, "Espulso dal server");
							deallocResources();
							return 0;
						}
						else if(strchr(message->parameters[0],'D')!=NULL) //server chiuso
						{
							print( ERROR, "Il server e' stato chiuso");
							deallocResources();
							return 0;
						}
						else if(strchr(message->parameters[0],'W')!=NULL) //risposta sbagliata
						{
							print( GAME, "Risposta Sbagliata!");
							sprintf(stringBuffer, "Ora hai %s punti",message->parameters[2]);
							print( INFO, stringBuffer);
							pthread_mutex_unlock(&mutex);
						}
						else if(strchr(message->parameters[0],'C')!=NULL) //risposta giusta
						{
							print( GAME, "Risposta Corretta!\n");
							sprintf(stringBuffer, "Ora hai %s punti",message->parameters[2]);
							print( INFO, stringBuffer);
						}
						else if(strchr(message->parameters[0],'T')!=NULL) //risposta giusta ma in ritardo
						{
							print( GAME, "Qualcuno ha risposto correttamente prima di te!");
							sprintf(stringBuffer, "Ora hai %s punti",message->parameters[2]);
							print( INFO, stringBuffer);
						}
						else if(strchr(message->parameters[0],'Q')!=NULL) //nuova domanda
						{
							setNewQuestion(message);
							if(waitingForUserInput==1 && testRun==0) //se il thread è bloccato in attesa di una risposta dall utente
							{
								waitingForUserInput=0;
								if (pthread_cancel(bash)!=0)//lo chiudo
								{
									print( ERROR, "Impossibile terminare il thread bash");
								}
								
								//e lo ricreo
								pthread_create (&bash, NULL, &userInput, &arg);
							}
							else //altrimenti lo sblocco semplicemente
							{
								pthread_mutex_unlock(&mutex);
							}
						}
						else if(strchr(message->parameters[0],'N')!=NULL) //messaggio di notifica
						{
							sprintf(stringBuffer, "%s",message->parameters[1]);
							print( INFO, stringBuffer);
							if(waitingForUserInput==1 && endGame==0 && testRun==0) //se il thread è bloccato in attesa di una risposta dall utente
							{
								waitingForUserInput=0;
								if (pthread_cancel(bash)!=0)//lo chiudo
								{
									print( ERROR, "Impossibile terminare il thread bash");
								}
								
								//e lo ricreo
								pthread_create (&bash, NULL, &userInput, &arg);
							}
						}
						else if(strchr(message->parameters[0],'R')!=NULL) //fine partita e classifica
						{
							printRanking(&(message->parameters[1]),(message->parameterCount)-2);
							/*print( GAME, "La partita si e' conclusa\nNome Punteggio\n----------------\n");
							endGame=1;
							sprintf(stringBuffer, "%s\n",message->parameters[1]);
							print( DEFAULT, stringBuffer);*/
							if(testRun==0)
							{
								if (pthread_cancel(bash)!=0) //chiudo il thread bash
								{
									print( ERROR, "Impossibile terminare il thread bash");
								}
							}
							deallocResources();	
							print( DEFAULT, "\n");					
							exit(EXIT_FAILURE);	
						}
						else
						{
							sprintf(stringBuffer, "Messaggio sconosciuto ricevuto: %s ",message->parameters[0]);
							print( ERROR, stringBuffer);
							//deallocResources();
							//return 0;
						}
					
					}
					
				}
				else // altrimenti se ricevo un errore mi chiudo preventivamente
				{
					print( ERROR, "Errore in lettura messaggio server\n");
					deallocResources();
					exit(EXIT_FAILURE);
				}
			}
		}
	} else {
		deallocResources();
		print( ERROR, "Errore richiesta autorizzazione\n");
		exit(EXIT_FAILURE);
	}
	return 0;
}
