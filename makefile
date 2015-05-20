#Elenco delle regole non corrispondenti ad un file fisico (per non aggiungere dipendenze alla regola)
.PHONY: all default bin assets game_wrapper assets_wrapper game_dirs assets_dirs log_dir game_clean assets_clean clean objects

#Compilatore usato
CC=gcc

#<----------------------------------------
TEST_CLIENT=5
TEST_WIN_POINTS=15

#Path dei vari files del progetto
COMMON=src/common
SERVER=src/server
CLIENT=src/client
TEST=src/test

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
	./bin/test/testGenerator $(TEST_CLIENT) $(TEST_WIN_POINTS)

test:
	@make bin
	@make assets
	@make launchClients.o
	$(CC) $(CFLAGS) bin/test/launchClients.o -o bin/test/launchClients
	@make assets_clean
	./bin/startGame --server --test > logs/serverOutput.log &

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
	$(CC) $(CFLAGS) bin/game/server.o bin/game/serverlib.o bin/game/commonlib.o -o bin/game/server
	$(CC) $(CFLAGS) bin/game/client.o bin/game/clientlib.o bin/game/commonlib.o -o bin/game/client
	$(CC) $(CFLAGS) bin/startGame.o -o bin/startGame

assets_objects: testGenerator.o
	@echo $(NOTIFY_STRING) Compilazione files oggetto
	$(CC) $(CFLAGS) bin/test/testGenerator.o -o bin/test/testGenerator

#Creazione della cartella bin con controllo della sua esistenza per non generare errori in caso sia già presente
game_dirs:
	@echo $(NOTIFY_STRING) Controllo della presenza cartella $(OK_COLOR)bin$(NO_COLOR)
	@if [ -d "bin" ]; then \
	echo $(WARN_STRING) La cartella $(OK_COLOR)bin$(NO_COLOR) esiste già; \
	else \
	echo $(NOTIFY_STRING) Creazione cartella $(OK_COLOR)bin$(NO_COLOR); \
	mkdir -p bin; \
	fi

	@echo $(NOTIFY_STRING) Controllo della presenza cartella $(OK_COLOR)bin/game$(NO_COLOR)
	@if [ -d "bin/game" ]; then \
	echo $(WARN_STRING) La cartella $(OK_COLOR)bin/game$(NO_COLOR) esiste già; \
	else \
	echo $(NOTIFY_STRING) Creazione cartella $(OK_COLOR)bin/game$(NO_COLOR) ; \
	mkdir -p bin/game ; \
	fi
	@echo $(NOTIFY_STRING) Creazione files oggetto

assets_dirs:
	@echo $(NOTIFY_STRING) Controllo della presenza cartella $(OK_COLOR)bin/test$(NO_COLOR)
	@if [ -d "bin/test" ]; then \
	echo $(WARN_STRING) La cartella $(OK_COLOR)bin/test$(NO_COLOR) esiste già; \
	else \
	echo $(NOTIFY_STRING) Creazione cartella $(OK_COLOR)bin/test$(NO_COLOR); \
	mkdir -p bin/test; \
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
	$(CC) -c $(COMMON)/commonlib.c -o bin/game/commonlib.o 2> logs/gcc.log
	
#Compilazione della libreria server senza linking nella cartella bin/game/
serverlib.o:
	$(CC) -c $(SERVER)/serverlib.c -o bin/game/serverlib.o 2> logs/gcc.log

#Compilazione del file server senza linking nella cartella bin/game/
server.o:
	$(CC) -c $(SERVER)/server.c -o bin/game/server.o 2> logs/gcc.log

#Compilazione della libreria client senza linking nella cartella bin/game/
clientlib.o:
	$(CC) -c $(CLIENT)/clientlib.c -o bin/game/clientlib.o 2> logs/gcc.log

#Compilazione del file client senza linking nella cartella bin/game/
client.o:
	$(CC) -c $(CLIENT)/client.c -o bin/game/client.o 2> logs/gcc.log

#Compilazione del file startGame senza linking nella cartella bin/
startGame.o:
	$(CC) -c $(COMMON)/startGame.c -o bin/startGame.o 2> logs/gcc.log

#Compilazione del file launchClients senza linking nella cartella bin/test/
launchClients.o:
	$(CC) -c $(TEST)/launchClients.c -o bin/test/launchClients.o 2> logs/gcc.log

#Compilazione del file testGenerator senza linking nella cartella bin/test/
testGenerator.o:
	$(CC) -c $(TEST)/testGenerator.c -o bin/test/testGenerator.o 2> logs/gcc.log

game_clean:
	rm bin/game/*.o
	rm bin/*.o

assets_clean:
	rm bin/test/*.o

clean: 
	@make game_clean 
	@make assets_clean

delete_assets:
	rm -rf assets/client/*
	rm -rf assets/server/*
