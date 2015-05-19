#Elenco delle regole non corrispondenti ad un file fisico (per non aggiungere dipendenze alla regola)
.PHONY: all clean default bin bin_dir

#Compilatore usato
CC=gcc

#Path dei vari files del progetto
COMMON=src/common
SERVER=src/server
CLIENT=src/client

#Files oggetto da usare per la compilazione del programma
OBJECTS=serverlib.o clientlib.o commonlib.o client.o server.o startGame.o

#Flags usate nella compilazione, in questo caso -pthread permette di utilizzare i threads
CFLAGS=-pthread

#Colori usati nei messaggi a schermo
NO_COLOR="\033[0m"
OK_COLOR="\033[32;1m"
ERROR_COLOR="\033[31;1m"
WARN_COLOR="\033[33;1m"

#Stringhe preimpostate per notifiche varie [MESSAGGIO]
NOTIFY_STRING=$(NO_COLOR)[$(OK_COLOR)NOTIFY$(NO_COLOR)]
WARN_STRING=$(NO_COLOR)[$(WARN_COLOR)WARNING$(NO_COLOR)]
ERROR_STRING=$(NO_COLOR)[$(ERROR_COLOR)ERROR$(NO_COLOR)]
FAIL_STRING = $(NO_COLOR)[$(ERROR_COLOR)FAIL$(NO_COLOR)]

#Con make si visualizza il messaggio di descrizione del progetto e le flag da usare per compilare/pulire/ecc
default:
	@clear
	@clear
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
	@echo Descrizione progetto:
	@echo "    Gioco multiplayer locale in cui un server invia operazioni" 
	@echo "    matematiche ai client e ne aspetta le risposte assegnando "
	@echo "    o togliendo punti in base alla loro correttezza"
	@echo
	@echo
	@echo Opzioni makefile
	@echo "   "$(ERROR_COLOR)"bin"$(NO_COLOR)":    Compila i files e li rende disponibili"
	@echo "           nella cartella bin/. Viene creato inoltre il file build.log"
	@echo "   "$(ERROR_COLOR)"clean"$(NO_COLOR)":  Pulisce il progetto eliminando i file oggetto"
	@echo "           clean viene chiamato automaticamente con l'uso di make bin"
	@echo "   "$(ERROR_COLOR)"objects"$(NO_COLOR)": Compila i files e li rende disponibili"
	@echo "           nella cartella bin/ SENZA richiamare la funzione clean"


#Per la creazione del file build.log ho creato un wrapper per non dover far scrivere all'utente make bin |tee a.log 
bin:
	@make wrapper | tee a.log #salva l'output dello schermo in un file di log
	@ cat a.log  | grep -v '\[' > build.log #Elaboro a.log per rimuovere le frasi informative che contengono [] ([NOTIFY], [WARN] ecc)
	@ rm a.log


wrapper: objects
	@echo $(NOTIFY_STRING) Pulizia files residui
	@make clean
	@echo $(NOTIFY_STRING) Fine

#Creazione della cartella bin con controllo della sua esistenza per non generare errori in caso sia già presente
bin_dir:
	@clear
	date
	@echo $(NOTIFY_STRING) Controllo della presenza cartella $(OK_COLOR)bin$(NO_COLOR)
	@if [ -d "bin" ]; then \
	echo $(WARN_STRING) La cartella $(OK_COLOR)bin$(NO_COLOR) esiste già; \
	fi
	@echo $(NOTIFY_STRING) Creazione cartella $(OK_COLOR)bin$(NO_COLOR)
	mkdir -p bin
	@echo $(NOTIFY_STRING) Creazione files oggetto

#Creazione dei file linkati client server e startGame, necessari i file oggetto (*.o)
objects: bin_dir $(OBJECTS)
	@echo $(NOTIFY_STRING) Compilazione files oggetto
	$(CC) $(CFLAGS) bin/server.o bin/serverlib.o bin/commonlib.o -o bin/server
	$(CC) $(CFLAGS) bin/client.o bin/clientlib.o bin/commonlib.o -o bin/client
	$(CC) $(CFLAGS) bin/startGame.o -o bin/startGame

#Compilazione della libreria comune senza linking nella cartella bin/
commonlib.o:
	$(CC) -c $(COMMON)/commonlib.c -o bin/commonlib.o
	
#Compilazione della libreria server senza linking nella cartella bin/
serverlib.o:
	$(CC) -c $(SERVER)/serverlib.c -o bin/serverlib.o

#Compilazione del file server senza linking nella cartella bin/
server.o:
	$(CC) -c $(SERVER)/server.c -o bin/server.o

#Compilazione della libreria client senza linking nella cartella bin/
clientlib.o:
	$(CC) -c $(CLIENT)/clientlib.c -o bin/clientlib.o

#Compilazione del file client senza linking nella cartella bin/
client.o:
	$(CC) -c $(CLIENT)/client.c -o bin/client.o

#Compilazione del file startGame senza linking nella cartella bin/
startGame.o:
	$(CC) -c $(COMMON)/startGame.c -o bin/startGame.o

#Regola per la rimozione dei files residui (files oggetto nella cartella bin)
clean:
	rm bin/*.o