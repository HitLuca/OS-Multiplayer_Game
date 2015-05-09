CC=gcc

COMMON=src/common
SERVER=src/server
CLIENT=src/client

CFLAGS=-pthread


default:
	clear
	@echo "                     ################################"
	@echo "                     # Progetto Sistemi Operativi 1 #"
	@echo "                     #       IN SWAG WE TRUST       #"
	@echo "                     ################################"
	@echo
	@echo "                               Studenti"
	@echo "                        Simonetto | Federici"
	@echo "                             Luca | Marco"
	@echo "                           166540 | 165183"
	@echo
	@echo
	@echo Opzioni makefile
	@echo "   bin:    Compila i files e li rende disponibili"
	@echo "           nella cartella bin/"
	@echo "   clean:  Pulisce il progetto eliminando i file oggetto"
	@echo "           clean viene chiamato automaticamente con l'uso di make bin"


bin: bin_dir serverlib.o clientlib.o commonlib.o client.o server.o
	#@echo [NOTIFY]:Compilazione dei files(*.o)
	$(CC) $(CFLAGS) bin/server.o bin/serverlib.o bin/commonlib.o -o bin/server
	$(CC) $(CFLAGS) bin/client.o bin/clientlib.o bin/commonlib.o -o bin/client
	#@echo [NOTIFY]:Avvio pulizia files residui
	make clean
	#@echo [NOTIFY]: Fine esecuzione makefile

bin_dir:
	#@echo [NOTIFY]:Creazione cartella bin/
	mkdir bin
	#@echo [NOTIFY]:Creazione file oggetto (*.o)

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