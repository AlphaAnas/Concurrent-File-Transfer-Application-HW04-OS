server: server.c
	gcc -o server server.c -lssl -lcrypto
client: client.c
	gcc -o client client.c -lssl -lcrypto
clean:
	rm -f server client
	rm -f out_*

all: server client
	