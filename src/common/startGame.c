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

void check_side(int argc, char** argv);
void server_side(int argc, char** argv);
void client_side();
void print_usage();

int main (int argc, char **argv)
{
    if (argc==1)
    {
        printf("Usage: startApplication --server/client\n");
    }
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
                server_side(argc, argv);
                break;
            }
            case 'c' : {
                client_side();
                break;
            }
            default: print_usage(); 
                exit(EXIT_FAILURE);
        }
    }
}


void server_side(int argc, char** argv) {
    int max=0;
    int win=0;
    int opt=0;

    static struct option server_options[] = {
        {"max",    required_argument, 0,  'm' },
        {"win",   required_argument, 0,  'w' },
    };

    int long_index =1;
    while ((opt = getopt_long(argc, argv,"mw:", server_options, &long_index )) != -1) {
        switch (opt) {
            case 'm' : max = atoi(optarg);
                break;
            case 'w' : win = atoi(optarg);
                break;
            default: 
                exit(EXIT_FAILURE);
        }
    }
    execl("./server", "./server", max, win, (char *) NULL);
}


void client_side() {
    execl("./client", "./client", (char *) NULL);
}

void print_usage() {
printf("Usage for server: startApplication --server --max n --win m (--max and --win optional)\n");
printf("Usage for client: startApplication --client\n");
} 