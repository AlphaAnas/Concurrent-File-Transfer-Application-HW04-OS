server: server.c
	gcc -o server server.c -lssl -lcrypto -Wall
client: client.c
	gcc -o client client.c -lssl -lcrypto -Wall
clean:
	rm -f server client
	rm -f out_*

all: server client
	