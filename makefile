#Elenco delle regole non corrispondenti ad un file fisico (per non aggiungere dipendenze alla regola)
.PHONY: all default bin assets game_wrapper assets_wrapper test_wrapper game_dirs assets_dirs log_dir game_clean assets_clean clean objects revert

#Compilatore usato
CC=gcc

#valori di default per la fase di testing
TEST_CLIENT=5
TEST_WIN_POINTS=15

#Variabile contenente il risultato del check delle differenze tra output server e output aspettato
#Usata con il comando make test
DIFF=$(diff assets/server/server.log log/server/server.log)

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
	@echo $(OK_COLOR)"  ______           "$(ERROR_COLOR)" _______        "$(WARN_COLOR)"                   _                   "$(NO_COLOR)
	@echo $(OK_COLOR)" / _____)          "$(ERROR_COLOR)"/  ___  \       "$(WARN_COLOR)"                  (_)              _   "$(NO_COLOR)
	@echo $(OK_COLOR)"( (____  _   _  ___"$(ERROR_COLOR)"| |   | |____   "$(WARN_COLOR)"  ____   ____ ___  _ _____  ____ _| |_ "$(NO_COLOR)
	@echo $(OK_COLOR)" \____ \| | | |/___"$(ERROR_COLOR)"| |   | |  _ \  "$(WARN_COLOR)" |  _ \ / ___) _ \| | ___ |/ ___|_   _)"$(NO_COLOR)
	@echo $(OK_COLOR)" _____) ) |_| |___ "$(ERROR_COLOR)"| |___| | |_| | "$(WARN_COLOR)" | |_| | |  | |_| | | ____( (___  | |_ "$(NO_COLOR)
	@echo $(OK_COLOR)"(______/ \__  (___/"$(ERROR_COLOR)" \_____/|  __/  "$(WARN_COLOR)" |  __/|_|   \___/| |_____)\____) \___)"$(NO_COLOR)
	@echo $(OK_COLOR)"        (____/     "$(ERROR_COLOR)"        |_|     "$(WARN_COLOR)" |_|            (__/                   "$(NO_COLOR)
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
	@echo "           nella cartella bin. Vengono loggate le istruzioni eseguite"
	@echo "           nella cartella log"
	@echo "   "$(ERROR_COLOR)"clean"$(NO_COLOR)":  Pulisce il progetto eliminando i file oggetto"
	@echo "           delle chiamate di make assets e make bin."
	@echo "           Clean viene chiamato automaticamente"
	@echo "           con l'uso di make bin e make assets"
	@echo "   "$(ERROR_COLOR)"objects"$(NO_COLOR)":Compila i files di gioco e li rende disponibili"
	@echo "           nella cartella bin SENZA richiamare la funzione clean"
	@echo "   "$(ERROR_COLOR)"assets"$(NO_COLOR)": Crea la cartella assets, contenente i files da"
	@echo "           utilizzare per la fase di testing del programma"
	@echo "   "$(ERROR_COLOR)"test"$(NO_COLOR)":   Avvia il programma in modalità testing, utilizzando i"
	@echo "           files contenuti nella cartella assets"
	@echo "   "$(ERROR_COLOR)"intensive_test"$(NO_COLOR)": Avvia il programma in modalità testing intensivo"
	@echo "           con domande e risposte casuali. Richiede però più"
	@echo "           tempo, a favore della precisione"
	@echo "   "$(ERROR_COLOR)"revert"$(NO_COLOR)": Elimina tutti i files creati dal makefile e eventuali"
	@echo "           files di log e asset, ricreando la gerarchia di"
	@echo "           cartelle iniziale"

#compila i files e li rende disponibili nella cartella bin bin
#Per la creazione del file build.log ho creato un wrapper per non dover far scrivere all'utente make bin |tee a.log 
bin: date_write log_dir
	@make game_wrapper | tee log/game_wrapper.log #salva l'output dello schermo in un file di log
	@ cat log/game_wrapper.log | grep -v '\[' > log/game_build.log #Elaboro game_wrapper.log per rimuovere le frasi informative che contengono [] ([NOTIFY], [WARN] ecc)
	@ rm log/game_wrapper.log
	@make check_logs

#Crea gli assset da usare
assets: date_write log_dir assets_dirs
	@make assets_wrapper | tee log/assets_wrapper.log
	@cat log/assets_wrapper.log | grep -v '\[' > log/assets_build.log
	@rm log/assets_wrapper.log
	@make check_logs

#crea i files di asset da usare per il test intensivo
intensive_assets: date_write log_dir assets_dirs
	@make intensive_assets_wrapper | tee log/intensive_assets_wrapper.log
	@cat log/intensive_assets_wrapper.log | grep -v '\[' > log/intensive_assets_build.log
	@rm log/intensive_assets_wrapper.log
	@make check_logs

#Come make test ma chiama intensive_assets al posto di assets
intensive_test: log_dir
	@make bin
	@make assets
	@make test_wrapper | tee log/test_wrapper.log
	@cat log/test_wrapper.log | grep -v '\[' > log/test_build.log
	@rm log/test_wrapper.log
	@make check_logs
	@echo $(NOTIFY_STRING) Avvio del server per il testing
	$(BIN)/startGame --server --win $(TEST_WIN_POINTS) --max $(TEST_CLIENT) --test

	@echo
	@#Controllo delle differenze tra output attuale e previsto
	@echo $(WARN_STRING) Controllo delle differenze tra output attuale e previsto...
	@if [ "$(DIFF)" != "" ]; then \
	echo $(ERROR_STRING) i files $(ERROR_COLOR)NON COINCIDONO$(NO_COLOR); \
	else \
	echo $(NOTIFY_STRING) i files $(OK_COLOR)COINCIDONO$(NO_COLOR); \
	fi

#Chiama bin e assets e poi avvia il testing del programma
test: log_dir
	@make bin
	@make intensive_assets
	@make test_wrapper | tee log/test_wrapper.log
	@cat log/test_wrapper.log | grep -v '\[' > log/intensive_test_build.log
	@rm log/test_wrapper.log
	@make check_logs
	@echo $(NOTIFY_STRING) Avvio del server per il testing
	$(BIN)/startGame --server --win $(TEST_WIN_POINTS) --max $(TEST_CLIENT) --test

#Fa il revert allo stato iniziale del progetto
revert:
	@echo $(NOTIFY_STRING) Inizio revert del progetto
	rm -rf bin
	rm -rf assets
	rm -rf log
	@echo $(NOTIFY_STRING) Fine revert

#Scrive la data
date_write:
	date

#Wrapper per make bin
game_wrapper: 
	@clear
	@make game_objects
	@echo $(NOTIFY_STRING) Pulizia files residui
	@make game_clean
	@echo $(NOTIFY_STRING) Fine

#Wrapper per gli assets
assets_wrapper: 
	@make assets_objects
	@echo $(NOTIFY_STRING) Pulizia files residui
	@make assets_clean
	@echo $(NOTIFY_STRING) Fine
	@make delete_assets
	@echo $(NOTIFY_STRING) Creazione degli assets
	./$(TEST_BIN)/testGenerator $(TEST_CLIENT) $(TEST_WIN_POINTS)
	@echo $(NOTIFY_STRING) Assets creati

#Wrapper per intensive_test
intensive_assets_wrapper:
	@make assets_objects
	@echo $(NOTIFY_STRING) Pulizia files residui
	@make assets_clean
	@echo $(NOTIFY_STRING) Fine
	@make delete_assets
	@echo $(NOTIFY_STRING) Creazione degli assets
	./$(TEST_BIN)/testGenerator $(TEST_CLIENT) 0
	@echo $(NOTIFY_STRING) Assets creati

#Wrapper per test
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

#Creazione del file linkato testGenerator, necessari i file oggetto (*.o)
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

#Creazione delle cartelle assets, assets/client e assets/server
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

#Creazione delle cartelle log, log/server e log/client
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

#Cancella i files oggetto nella cartella bin e bin/game
game_clean:
	rm $(GAME_BIN)/*.o
	rm $(BIN)/*.o

#Cancella i files oggetto nella cartella bin/test
assets_clean:
	rm $(TEST_BIN)/*.o

#Chiama i clean
clean: 
	@make game_clean 
	@make assets_clean

#Cancella gli assets client e server
delete_assets:
	rm -rf assets/client/*
	rm -rf assets/server/*

#Controlla che il file log/gcc.log non sia vuoto, sennò lo cancella
check_logs:
	@echo $(NOTIFY_STRING) Rimozione files di log non utilizzati
	@if [ -s log/gcc.log ] ; then \
	echo; \
	else \
	rm -rf log/gcc.log; \
	fi

#Non penso serva più
#objects: game_dirs $(GAME_OBJECTS)
#@echo $(NOTIFY_STRING) Fine