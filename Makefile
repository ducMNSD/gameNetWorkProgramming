all:
	gcc -o server server.c
	gcc -o client client.c -pthread `pkg-config gtk+-3.0 --cflags --libs`
	clear