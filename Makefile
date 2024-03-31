CC_FLAG=-O2 -Wall 
CC=gcc

all: 
	if [ ! -d "out" ]; then mkdir out; fi

	$(CC) $(CC_FLAG) server.c -o out/server
	$(CC) $(CC_FLAG) client.c -o out/client