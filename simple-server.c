#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    

    char server_message[256] = "You have reached the server!";
    //create a socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        error("Error opening socket");
    }

    //specify an address for the socket
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8080);
    server_address.sin_addr.s_addr = INADDR_ANY;// shortcut for IP address:

    //bind the socket to our specified IP and port
    bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address));

    listen(server_socket, 5); // 5 is the number of connections that can be waiting while the server is busy

    int client_socket;           
    client_socket = accept(server_socket, NULL, NULL);


    //send the message
    send(client_socket, server_message, sizeof(server_message), 0);


    //close the socket
    close(server_socket);

    return 0;
}
