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
	int i;
	for(i=1;i<=10;i++)
	{
		if(fork()==0)
		{
			char index[20];	
			sprintf(index,"%d",i);
			execl("./client","./client",index,(char*)0);
		}
	}
}
