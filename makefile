#Elenco delle regole non corrispondenti ad un file fisico (per non aggiungere dipendenze alla regola)
.PHONY: all default bin assets game_wrapper assets_wrapper game_dirs assets_dirs log_dir game_clean assets_clean clean objects

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
	@echo Descrizione progetto:
	@echo "    Gioco multiplayer locale in cui un server invia operazioni" 
	@echo "    matematiche ai client e ne aspetta le risposte assegnando "
	@echo "    o togliendo punti in base alla loro correttezza"
	@echo
	@echo Opzioni makefile
	@echo "   "$(ERROR_COLOR)"bin"$(NO_COLOR)":    Compila i files e li rende disponibili"
	@echo "           nella cartella bin/. Viene creato inoltre il file build.log e gcc.log nella cartella logs"
	@echo "   "$(ERROR_COLOR)"clean"$(NO_COLOR)":  Pulisce il progetto eliminando i file oggetto"
	@echo "           clean viene chiamato automaticamente con l'uso di make bin"
	@echo "   "$(ERROR_COLOR)"objects"$(NO_COLOR)": Compila i files e li rende disponibili"
	@echo "           nella cartella bin/ SENZA richiamare la funzione clean"
	@echo "   "$(ERROR_COLOR)"assets"$(NO_COLOR)": Crea la cartella assets, contenente i files da"
	@echo "           utilizzare per la fase di testing del programma"
	@echo "   "$(ERROR_COLOR)"test"$(NO_COLOR)": Avvia il programma in modalità testing, utilizzando i"
	@echo "           files contenuti nella cartella assets"

#Per la creazione del file build.log ho creato un wrapper per non dover far scrivere all'utente make bin |tee a.log 
bin: log_dir
	@make game_wrapper | tee logs/game_wrapper.log #salva l'output dello schermo in un file di log
	@ cat logs/game_wrapper.log | grep -v '\[' > logs/game_build.log #Elaboro game_wrapper.log per rimuovere le frasi informative che contengono [] ([NOTIFY], [WARN] ecc)
	@ rm logs/game_wrapper.log

assets: log_dir assets_dirs
	@make assets_wrapper | tee logs/assets_wrapper.log
	@cat logs/assets_wrapper.log | grep -v '\[' > logs/assets_build.log
	@rm logs/assets_wrapper.log
	@make delete_assets
	@echo $(NOTIFY_STRING) Creazione degli assets
	./$(TEST_BIN)/testGenerator $(TEST_CLIENT) $(TEST_WIN_POINTS)
	@echo $(NOTIFY_STRING) Assets creati

test:
	@make bin
	@make assets
	@make launchClients.o
	$(CC) $(CFLAGS) $(TEST_BIN)/launchClients.o -o $(TEST_BIN)/launchClients 2> logs/gcc.log
	@make assets_clean
	@echo $(NOTIFY_STRING) Avvio del server per il testing
	$(BIN)/startGame --server --win $(TEST_WIN_POINTS) --max $(TEST_CLIENT) --test

game_wrapper: 
	@clear
	date
	@make game_objects
	@echo $(NOTIFY_STRING) Pulizia files residui
	@make game_clean
	@echo $(NOTIFY_STRING) Fine

assets_wrapper: 
	@make assets_objects
	@echo $(NOTIFY_STRING) Pulizia files residui
	@make assets_clean
	@echo $(NOTIFY_STRING) Fine

#Creazione dei file linkati client server e startGame, necessari i file oggetto (*.o)
game_objects:  game_dirs $(GAME_OBJECTS)
	@echo $(NOTIFY_STRING) Compilazione files oggetto
	$(CC) $(CFLAGS) $(GAME_BIN)/server.o $(GAME_BIN)/serverlib.o $(GAME_BIN)/commonlib.o -o $(GAME_BIN)/server 2> logs/gcc.log
	$(CC) $(CFLAGS) $(GAME_BIN)/client.o $(GAME_BIN)/clientlib.o $(GAME_BIN)/commonlib.o -o $(GAME_BIN)/client 2> logs/gcc.log
	$(CC) $(CFLAGS) $(BIN)/startGame.o -o $(BIN)/startGame 2> logs/gcc.log

assets_objects: testGenerator.o
	@echo $(NOTIFY_STRING) Compilazione files oggetto
	$(CC) $(CFLAGS) $(TEST_BIN)/testGenerator.o -o $(TEST_BIN)/testGenerator 2> logs/gcc.log

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
	@echo $(NOTIFY_STRING) Controllo della presenza cartella $(OK_COLOR)logs$(NO_COLOR)
	@if [ -d "logs" ]; then \
	echo $(WARN_STRING) La cartella $(OK_COLOR)logs$(NO_COLOR) esiste già; \
	else \
	echo $(NOTIFY_STRING) Creazione cartella $(OK_COLOR)logs$(NO_COLOR); \
	mkdir -p logs; \
	fi

objects: game_dirs $(GAME_OBJECTS)
	@echo $(NOTIFY_STRING) Fine

#Compilazione della libreria comune senza linking nella cartella bin/game/
commonlib.o:
	$(CC) -c $(COMMON_SRC)/commonlib.c -o $(GAME_BIN)/commonlib.o 2> logs/gcc.log
	
#Compilazione della libreria server senza linking nella cartella bin/game/
serverlib.o:
	$(CC) -c $(SERVER_SRC)/serverlib.c -o $(GAME_BIN)/serverlib.o 2> logs/gcc.log

#Compilazione del file server senza linking nella cartella bin/game/
server.o:
	$(CC) -c $(SERVER_SRC)/server.c -o $(GAME_BIN)/server.o 2> logs/gcc.log

#Compilazione della libreria client senza linking nella cartella bin/game/
clientlib.o:
	$(CC) -c $(CLIENT_SRC)/clientlib.c -o $(GAME_BIN)/clientlib.o 2> logs/gcc.log

#Compilazione del file client senza linking nella cartella bin/game/
client.o:
	$(CC) -c $(CLIENT_SRC)/client.c -o $(GAME_BIN)/client.o 2> logs/gcc.log

#Compilazione del file startGame senza linking nella cartella bin/
startGame.o:
	$(CC) -c $(COMMON_SRC)/startGame.c -o $(BIN)/startGame.o 2> logs/gcc.log

#Compilazione del file launchClients senza linking nella cartella bin/test/
launchClients.o:
	$(CC) -c $(TEST_SRC)/launchClients.c -o $(TEST_BIN)/launchClients.o 2> logs/gcc.log

#Compilazione del file testGenerator senza linking nella cartella bin/test/
testGenerator.o:
	$(CC) -c $(TEST_SRC)/testGenerator.c -o $(TEST_BIN)/testGenerator.o 2> logs/gcc.log

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
