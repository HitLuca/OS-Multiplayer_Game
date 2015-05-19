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

#define MAX_LIMIT 20
#define WIN_LIMIT 20
#define CLIENT_LIMIT 30

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

//Funzione di controllo (se lato server o lato client)
void check_side(int argc, char** argv) {
    int opt;

    int long_index=0;

    static struct option long_options[] = {
            {"server",    no_argument, 0,  's' },
            {"client",   no_argument, 0,  'c' },
        };

    if ((opt = getopt_long(argc, argv,"s::c::", long_options, &long_index )) != -1) {
        switch (opt) {
            case 's' : {
                server_side(argc, argv);
                break;
            }
            case 'c' : {
                client_side(argc, argv);
                break;
            }
            default: {
                print_usage(); 
                exit(EXIT_FAILURE);
            }
        }
    }
}

//Lettura eventuali argomenti server e avvio eseguibile lato server
void server_side(int argc, char** argv) {
    char max[3]="0";
    char win[3]="0";
    char test[3]="0";

    int iOptarg;
    int opt=0;

    static struct option server_options[] = {
        {"max",    required_argument, 0,  'm' },
        {"win",   required_argument, 0,  'w' },
        {"test", no_argument, 0, 't'},
    };

    int long_index =1;
    while ((opt = getopt_long(argc, argv,"m:w:t::", server_options, &long_index )) != -1) {
        switch (opt) {
            case 'm' :
            {
                iOptarg=atoi(optarg);
                if(iOptarg==0)
                {
                    printf("Errore, l'argomento di max è errato\n");
                    exit(EXIT_FAILURE);
                }
                else if (iOptarg<=MAX_LIMIT)
                {
                    sprintf(max, "%d", iOptarg);
                }
                else
                {
                    printf("Errore, l'argomento di max è troppo grande\n");
                    exit(EXIT_FAILURE);
                }
                break;
            }
            case 'w' :
            { 
                iOptarg=atoi(optarg);
                if(iOptarg==0)
                {
                    printf("Errore, l'argomento di win è errato\n");
                    exit(EXIT_FAILURE);
                }
                else if (iOptarg<=WIN_LIMIT)
                {
                    sprintf(win, "%d", iOptarg);
                    printf("%s", win);
                }
                else
                {
                    printf("Errore, l'argomento di win è troppo grande\n");
                    exit(EXIT_FAILURE);
                }
                break;
            }
            case 't' :
            {
                strcpy(test, "1");
                break;
            }
            default:
            {
                exit(EXIT_FAILURE);
            }
        }
    }
    execl("./server", "./server", max, win, test, (char *) NULL);
}

//Avvio dell'eseguibile lato client, senza argomenti
void client_side(int argc, char** argv) {
    char number[3]="0";
    int opt=0;
    int iOptarg;

    static struct option client_options[] = {
        {"test", required_argument, 0, 't'},
    };

    int long_index =1;
    while ((opt = getopt_long(argc, argv,"t:", client_options, &long_index )) != -1) {
        switch (opt) {
            case 't' :
            {
                iOptarg=atoi(optarg);
                if(iOptarg==0)
                {
                    printf("Errore, l'argomento di test è errato\n");
                    exit(EXIT_FAILURE);
                }
                else if (iOptarg<=CLIENT_LIMIT)
                {
                    sprintf(number, "%d", iOptarg);
                }
                else
                {
                    printf("Errore, l'argomento di test è troppo grande\n");
                    exit(EXIT_FAILURE);
                }
                break;
            }
            default:
            {
                exit(EXIT_FAILURE);
            }
        }
    }
    execl("./client", "./client", number, (char *) NULL);
}

//Funzione di notifica del corretto funzionamento del programma
void print_usage() {
    printf("Usage for server: startApplication --server --max n --win m (--max and --win optional)\n");
    printf("Usage for client: startApplication --client\n");
} 