#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>

int main(int argc,char** argv)
{
	char filePath[500];
    char* c;
    char* past;
    int clientNum = atoi(argv[1]);
    int maxPoints = atoi(argv[2]);
    strcpy(filePath, argv[0]);
    c=filePath;
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
    
    strcpy(past, "../game/client");
	int i;
	for(i=1;i<=clientNum;i++)
	{
		if(fork()==0)
		{
			char index[20];	
			sprintf(index,"%d",i);
			execl(filePath, filePath, index,"0","0",(char*)0);
		}
	}
	
	
	return 0;
	
}
