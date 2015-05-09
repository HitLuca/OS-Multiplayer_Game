CC=gcc

.PHONY: all

COMMON=src/common
SERVER=src/server
CLIENT=src/client

OBJECTS=serverlib.o clientlib.o commonlib.o client.o server.o

CFLAGS=-pthread

NO_COLOR="\033[0m"
OK_COLOR="\033[32;1m"
ERROR_COLOR="\033[31;1m"
WARN_COLOR="\033[33;1m"

NOTIFY_STRING=$(NO_COLOR)[$(OK_COLOR)NOTIFY$(NO_COLOR)]
WARN_STRING=$(NO_COLOR)[$(WARN_COLOR)WARNING$(NO_COLOR)]
ERROR_STRING=$(NO_COLOR)[$(ERROR_COLOR)ERROR$(NO_COLOR)]
FAIL_STRING = $(NO_COLOR)[$(ERROR_COLOR)FAIL$(NO_COLOR)]


default:
	clear
	@echo "                     "$(WARN_COLOR)"################################"$(NO_COLOR)
	@echo "                     "$(WARN_COLOR)"# Progetto Sistemi Operativi 1 #"$(NO_COLOR)
	@echo "                     "$(WARN_COLOR)"################################"$(NO_COLOR)
	@echo
	@echo "                               Studenti"
	@echo "                        "$(OK_COLOR)"Simonetto "$(NO_COLOR)"| "$(OK_COLOR)"Federici"$(NO_COLOR)
	@echo "                             "$(OK_COLOR)"Luca "$(NO_COLOR)"| "$(OK_COLOR)"Marco"$(NO_COLOR)
	@echo "                           "$(OK_COLOR)"166540 "$(NO_COLOR)"| "$(OK_COLOR)"165183"$(NO_COLOR)
	@echo
	@echo
	@echo Opzioni makefile
	@echo "   "$(ERROR_COLOR)"bin"$(NO_COLOR)":    Compila i files e li rende disponibili"
	@echo "           nella cartella bin/"
	@echo "   "$(ERROR_COLOR)"clean"$(NO_COLOR)":  Pulisce il progetto eliminando i file oggetto"
	@echo "           clean viene chiamato automaticamente con l'uso di make bin"
	@echo "   "$(ERROR_COLOR)"objects"$(NO_COLOR)": Compila i files e li rende disponibili"
	@echo "           nella cartella bin/ SENZA richiamare la funzione clean"


bin: objects
	@echo $(NOTIFY_STRING) Pulizia files residui
	@make clean
	@echo $(NOTIFY_STRING) Fine

bin_dir:
	clear
	@echo $(NOTIFY_STRING) Controllo della presenza cartella $(OK_COLOR)bin$(NO_COLOR)
	@if [ -d "bin" ]; then \
	echo $(WARN_STRING) La cartella $(OK_COLOR)bin$(NO_COLOR) esiste già; \
	fi
	@echo $(NOTIFY_STRING) Creazione cartella $(OK_COLOR)bin$(NO_COLOR)
	mkdir -p bin
	@echo $(NOTIFY_STRING) Creazione files oggetto

objects: bin_dir $(OBJECTS)
	@echo $(NOTIFY_STRING) Compilazione files oggetto
	$(CC) $(CFLAGS) bin/server.o bin/serverlib.o bin/commonlib.o -o bin/server
	$(CC) $(CFLAGS) bin/client.o bin/clientlib.o bin/commonlib.o -o bin/client

commonlib.o:
	$(CC) -c $(COMMON)/commonlib.c -o bin/commonlib.o
	
serverlib.o:
	$(CC) -c $(SERVER)/serverlib.c -o bin/serverlib.o
	
server.o:
	$(CC) -c $(SERVER)/server.c -o bin/server.o

clientlib.o:
	$(CC) -c $(CLIENT)/clientlib.c -o bin/clientlib.o

client.o:
	$(CC) -c $(CLIENT)/client.c -o bin/client.o

clean:
	rm bin/*.o