CC=gcc
CC_FLAG=-O2 -Wall
CC_DEBUG_FLAG=-g -O0 -Wall -static-libasan -no-pie

build: 
	if [ ! -d "out" ]; then mkdir out; fi

	$(CC) $(CC_FLAG) server.c -o out/server
	$(CC) $(CC_FLAG) client.c -o out/client

debug:

	if [ ! -d "out" ]; then mkdir out; fi

	$(CC) $(CC_DEBUG_FLAG) server.c -o out/server
	$(CC) $(CC_DEBUG_FLAG) client.c -o out/client