#Elenco delle regole non corrispondenti ad un file fisico (per non aggiungere dipendenze alla regola)
.PHONY: all default bin assets game_wrapper assets_wrapper test_wrapper game_dirs assets_dirs log_dir game_clean assets_clean clean objects revert

#Compilatore usato
CC=gcc

#<----------------------------------------
TEST_CLIENT=5
TEST_WIN_POINTS=15

#Variabile contenente il percorso assoluto al progetto
ROOT_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

#Path dei vari files del progetto
COMMON_SRC=src/common
SERVER_SRC=src/server
CLIENT_SRC=src/client
TEST_SRC=src/test
GAME_BIN=bin/game
TEST_BIN=bin/test
BIN=bin

#Files oggetto da usare per la compilazione del programma
GAME_OBJECTS=serverlib.o clientlib.o commonlib.o client.o server.o startGame.o
ASSETS_OBJECTS=launchClients.o testGenerator.o

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
	@echo "                  "$(WARN_COLOR)"################################"$(NO_COLOR)
	@echo "                  "$(WARN_COLOR)"# Progetto Sistemi Operativi 1 #"$(NO_COLOR)
	@echo "                  "$(WARN_COLOR)"################################"$(NO_COLOR)
	@echo
	@echo "                               Studenti"
	@echo "                        "$(OK_COLOR)"Simonetto "$(NO_COLOR)"| "$(OK_COLOR)"Federici"$(NO_COLOR)
	@echo "                             "$(OK_COLOR)"Luca "$(NO_COLOR)"| "$(OK_COLOR)"Marco"$(NO_COLOR)
	@echo "                           "$(OK_COLOR)"166540 "$(NO_COLOR)"| "$(OK_COLOR)"165183"$(NO_COLOR)
	@echo
	@echo $(WARN_COLOR)"Descrizione progetto:"$(NO_COLOR)
	@echo "    Gioco multiplayer locale in cui un server invia domande di vario" 
	@echo "    tipo ai client e ne aspetta le risposte assegnando o togliendo"
	@echo "    punti in base alla loro correttezza"
	@echo
	@echo $(WARN_COLOR)"Opzioni makefile:"$(NO_COLOR)
	@echo "   "$(ERROR_COLOR)"bin"$(NO_COLOR)":    Compila i files e li rende disponibili"
	@echo "           nella cartella bin. Vengono loggate le istruzioni eseguite nella"
	@echo "           cartella log"
	@echo "   "$(ERROR_COLOR)"clean"$(NO_COLOR)":  Pulisce il progetto eliminando i file oggetto"
	@echo "           delle chiamate di make assets e make bin. Clean viene chiamato"
	@echo "           automaticamente"
	@echo "           con l'uso di make bin e make assets"
	@echo "   "$(ERROR_COLOR)"objects"$(NO_COLOR)":Compila i files di gioco e li rende disponibili"
	@echo "           nella cartella bin SENZA richiamare la funzione clean"
	@echo "   "$(ERROR_COLOR)"assets"$(NO_COLOR)": Crea la cartella assets, contenente i files da"
	@echo "           utilizzare per la fase di testing del programma"
	@echo "   "$(ERROR_COLOR)"test"$(NO_COLOR)":   Avvia il programma in modalità testing, utilizzando i"
	@echo "           files contenuti nella cartella assets"
	@echo "   "$(ERROR_COLOR)"revert"$(NO_COLOR)": Elimina tutti i files creati dal makefile e eventuali"
	@echo "           files di log e asset, ricreando la gerarchia di cartelle iniziale"

#Per la creazione del file build.log ho creato un wrapper per non dover far scrivere all'utente make bin |tee a.log 
bin: date_write log_dir
	@make game_wrapper | tee log/game_wrapper.log #salva l'output dello schermo in un file di log
	@ cat log/game_wrapper.log | grep -v '\[' > log/game_build.log #Elaboro game_wrapper.log per rimuovere le frasi informative che contengono [] ([NOTIFY], [WARN] ecc)
	@ rm log/game_wrapper.log
	@make check_logs

assets: date_write log_dir assets_dirs
	@make assets_wrapper | tee log/assets_wrapper.log
	@cat log/assets_wrapper.log | grep -v '\[' > log/assets_build.log
	@rm log/assets_wrapper.log
	@make check_logs

test: log_dir
	@make bin
	@make assets
	@make test_wrapper | tee log/test_wrapper.log
	@cat log/test_wrapper.log | grep -v '\[' > log/test_build.log
	@rm log/test_wrapper.log
	@make check_logs
	@echo $(NOTIFY_STRING) Avvio del server per il testing
	$(BIN)/startGame --server --win $(TEST_WIN_POINTS) --max $(TEST_CLIENT) --test

revert:
	@echo $(NOTIFY_STRING) Inizio revert del progetto
	rm -rf bin
	rm -rf assets
	rm -rf log
	@echo $(NOTIFY_STRING) Fine revert

date_write:
	date

game_wrapper: 
	@clear
	@make game_objects
	@echo $(NOTIFY_STRING) Pulizia files residui
	@make game_clean
	@echo $(NOTIFY_STRING) Fine

assets_wrapper: 
	@make assets_objects
	@echo $(NOTIFY_STRING) Pulizia files residui
	@make assets_clean
	@echo $(NOTIFY_STRING) Fine
	@make delete_assets
	@echo $(NOTIFY_STRING) Creazione degli assets
	./$(TEST_BIN)/testGenerator $(TEST_CLIENT) $(TEST_WIN_POINTS)
	@echo $(NOTIFY_STRING) Assets creati

test_wrapper:
	@make launchClients.o
	$(CC) $(CFLAGS) $(TEST_BIN)/launchClients.o -o $(TEST_BIN)/launchClients 2> log/gcc.log
	@make assets_clean

#Creazione dei file linkati client server e startGame, necessari i file oggetto (*.o)
game_objects:  game_dirs $(GAME_OBJECTS)
	@echo $(NOTIFY_STRING) Compilazione files oggetto
	$(CC) $(CFLAGS) $(GAME_BIN)/server.o $(GAME_BIN)/serverlib.o $(GAME_BIN)/commonlib.o -o $(GAME_BIN)/server 2> log/gcc.log
	$(CC) $(CFLAGS) $(GAME_BIN)/client.o $(GAME_BIN)/clientlib.o $(GAME_BIN)/commonlib.o -o $(GAME_BIN)/client 2> log/gcc.log
	$(CC) $(CFLAGS) $(BIN)/startGame.o -o $(BIN)/startGame 2> log/gcc.log

assets_objects: testGenerator.o
	@echo $(NOTIFY_STRING) Compilazione files oggetto
	$(CC) $(CFLAGS) $(TEST_BIN)/testGenerator.o -o $(TEST_BIN)/testGenerator 2> log/gcc.log

#Creazione della cartella bin con controllo della sua esistenza per non generare errori in caso sia già presente
game_dirs:
	@echo $(NOTIFY_STRING) Controllo della presenza cartella $(OK_COLOR)$(BIN)$(NO_COLOR)
	@if [ -d "$(BIN)" ]; then \
	echo $(WARN_STRING) La cartella $(OK_COLOR)$(BIN)$(NO_COLOR) esiste già; \
	else \
	echo $(NOTIFY_STRING) Creazione cartella $(OK_COLOR)$(BIN)$(NO_COLOR); \
	mkdir -p $(BIN); \
	fi

	@echo $(NOTIFY_STRING) Controllo della presenza cartella $(OK_COLOR)$(GAME_BIN)$(NO_COLOR)
	@if [ -d "$(GAME_BIN)" ]; then \
	echo $(WARN_STRING) La cartella $(OK_COLOR)$(GAME_BIN)$(NO_COLOR) esiste già; \
	else \
	echo $(NOTIFY_STRING) Creazione cartella $(OK_COLOR)$(GAME_BIN)$(NO_COLOR) ; \
	mkdir -p $(GAME_BIN) ; \
	fi
	@echo $(NOTIFY_STRING) Creazione files oggetto

assets_dirs:
	@echo $(NOTIFY_STRING) Controllo della presenza cartella $(OK_COLOR)$(TEST_BIN)$(NO_COLOR)
	@if [ -d "$(TEST_BIN)" ]; then \
	echo $(WARN_STRING) La cartella $(OK_COLOR)$(TEST_BIN)$(NO_COLOR) esiste già; \
	else \
	echo $(NOTIFY_STRING) Creazione cartella $(OK_COLOR)$(TEST_BIN)$(NO_COLOR); \
	mkdir -p $(TEST_BIN); \
	fi

	@echo $(NOTIFY_STRING) Controllo della presenza cartella $(OK_COLOR)assets$(NO_COLOR)
	@if [ -d "assets" ]; then \
	echo $(WARN_STRING) La cartella $(OK_COLOR)assets$(NO_COLOR) esiste già; \
	else \
	echo $(NOTIFY_STRING) Creazione cartella $(OK_COLOR)assets$(NO_COLOR); \
	mkdir -p assets; \
	fi

	@echo $(NOTIFY_STRING) Controllo della presenza cartella $(OK_COLOR)assets/client$(NO_COLOR)
	@if [ -d "assets/client" ]; then \
	echo $(WARN_STRING) La cartella $(OK_COLOR)assets/client$(NO_COLOR) esiste già; \
	else \
	echo $(NOTIFY_STRING) Creazione cartella $(OK_COLOR)assets/client$(NO_COLOR); \
	mkdir -p assets/client; \
	fi

	@echo $(NOTIFY_STRING) Controllo della presenza cartella $(OK_COLOR)assets/server$(NO_COLOR)
	@if [ -d "assets/server" ]; then \
	echo $(WARN_STRING) La cartella $(OK_COLOR)assets/server$(NO_COLOR) esiste già; \
	else \
	echo $(NOTIFY_STRING) Creazione cartella $(OK_COLOR)assets/server$(NO_COLOR); \
	mkdir -p assets/server; \
	fi
	@echo $(NOTIFY_STRING) Creazione files oggetto

log_dir:
	@echo $(NOTIFY_STRING) Controllo della presenza cartella $(OK_COLOR)log$(NO_COLOR)
	@if [ -d "log" ]; then \
	echo $(WARN_STRING) La cartella $(OK_COLOR)log$(NO_COLOR) esiste già; \
	else \
	echo $(NOTIFY_STRING) Creazione cartella $(OK_COLOR)log$(NO_COLOR); \
	mkdir -p log; \
	fi

	@if [ -d "log/server" ]; then \
	echo $(WARN_STRING) La cartella $(OK_COLOR)log/server$(NO_COLOR) esiste già; \
	else \
	echo $(NOTIFY_STRING) Creazione cartella $(OK_COLOR)log/server$(NO_COLOR); \
	mkdir -p log/server; \
	fi

	@if [ -d "log/client" ]; then \
	echo $(WARN_STRING) La cartella $(OK_COLOR)log/client$(NO_COLOR) esiste già; \
	else \
	echo $(NOTIFY_STRING) Creazione cartella $(OK_COLOR)log/client$(NO_COLOR); \
	mkdir -p log/client; \
	fi

objects: game_dirs $(GAME_OBJECTS)
	@echo $(NOTIFY_STRING) Fine

#Compilazione della libreria comune senza linking nella cartella bin/game/
commonlib.o:
	$(CC) -c $(COMMON_SRC)/commonlib.c -o $(GAME_BIN)/commonlib.o 2> log/gcc.log
	
#Compilazione della libreria server senza linking nella cartella bin/game/
serverlib.o:
	$(CC) -c $(SERVER_SRC)/serverlib.c -o $(GAME_BIN)/serverlib.o 2> log/gcc.log

#Compilazione del file server senza linking nella cartella bin/game/
server.o:
	$(CC) -c $(SERVER_SRC)/server.c -o $(GAME_BIN)/server.o 2> log/gcc.log

#Compilazione della libreria client senza linking nella cartella bin/game/
clientlib.o:
	$(CC) -c $(CLIENT_SRC)/clientlib.c -o $(GAME_BIN)/clientlib.o 2> log/gcc.log

#Compilazione del file client senza linking nella cartella bin/game/
client.o:
	$(CC) -c $(CLIENT_SRC)/client.c -o $(GAME_BIN)/client.o 2> log/gcc.log

#Compilazione del file startGame senza linking nella cartella bin/
startGame.o:
	$(CC) -c $(COMMON_SRC)/startGame.c -o $(BIN)/startGame.o 2> log/gcc.log

#Compilazione del file launchClients senza linking nella cartella bin/test/
launchClients.o:
	$(CC) -c $(TEST_SRC)/launchClients.c -o $(TEST_BIN)/launchClients.o 2> log/gcc.log

#Compilazione del file testGenerator senza linking nella cartella bin/test/
testGenerator.o:
	$(CC) -c $(TEST_SRC)/testGenerator.c -o $(TEST_BIN)/testGenerator.o 2> log/gcc.log

game_clean:
	rm $(GAME_BIN)/*.o
	rm $(BIN)/*.o

assets_clean:
	rm $(TEST_BIN)/*.o

clean: 
	@make game_clean 
	@make assets_clean

delete_assets:
	rm -rf assets/client/*
	rm -rf assets/server/*

check_logs:
	@echo $(NOTIFY_STRING) Rimozione files di log non utilizzati
	@if [ -s log/gcc.log ] ; then \
	@echo; \
	else \
		rm -rf log/gcc.log; \
	fi