CC=gcc

COMMON=src/common
SERVER=src/server
CLIENT=src/client

CFLAGS=-pthread

bin: bin_dir serverlib.o clientlib.o commonlib.o client.o server.o
	$(CC) $(CFLAGS) bin/server.o bin/serverlib.o bin/commonlib.o -o bin/server
	$(CC) $(CFLAGS) bin/client.o bin/clientlib.o bin/commonlib.o -o bin/client
	make clean

bin_dir:
	mkdir bin

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