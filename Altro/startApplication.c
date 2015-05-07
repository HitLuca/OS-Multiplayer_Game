#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>

#include <ctype.h>

#define FIFO_NAME "fifo_risposte"

/*int main(int argc, char* argv[])
{
	char s[300];
    int maxclients = 10;
    int maxscore = 15;
    int num, fd, childpid;
    
    childpid=fork();

    if (childpid==0) {
        //Parte destinata a client authentication

    }
    else {
        //Parte destinata a ricezione risposte
    }

    mknod(FIFO_NAME, S_IFIFO | 0666, 0);

    printf("waiting for writers...\n");
    fd = open(FIFO_NAME, O_RDONLY);
    printf("got a player!\n");
	printf("Please write an operation: ");

    do {
        if ((num = read(fd, as, 300)) == -1)
            perror("read");
        else {
            s[num] = '\0';
            printf("tick: read %d bytes: \"%s\"\n", num, s);
        }
    } while (num > 0);
    return 0;
}*/  

void check_side(int argc, char** argv);
void server_side(int argc, char** argv);
void client_side();
void print_usage_server();

int main (int argc, char **argv)
{
    check_side(argc, argv);
    return 0;
}


    void check_side(int argc, char** argv) {
        int opt;

        int long_index=0;

        static struct option long_options[] = {
                {"server",    no_argument, 0,  's' },
                {"client",   no_argument, 0,  'c' },
            };

        if ((opt = getopt_long(argc, argv,"sc:", long_options, &long_index )) != -1) {
            switch (opt) {
                case 's' : {
                    printf("serverSide!\n"); 
                    server_side(argc, argv);
                    break;
                }
                case 'c' : {
                    printf("clientSide!\n");
                    client_side();
                    break;
                }
                default: print_usage_server(); 
                    exit(EXIT_FAILURE);
            }
        }
    }


    void server_side(int argc, char** argv) {
        int max, win;
        int opt=0;

        static struct option server_options[] = {
            {"max",    required_argument, 0,  'm' },
            {"win",   required_argument, 0,  'w' },
        };

        int long_index =1;
        while ((opt = getopt_long(argc, argv,"mw:", server_options, &long_index )) != -1) {
            switch (opt) {
                case 'm' : max = atoi(optarg);
                    printf("max=%d\n", max);
                    break;
                case 'w' : win = atoi(optarg);
                printf("win=%d\n", win);
                    break;
                default: 
                    exit(EXIT_FAILURE);
            }
        }
    }


    void client_side() {
    }

    void print_usage_server() {
    printf("Usage: ./multiplayer --server --max <max> --win <win>\n");
} 