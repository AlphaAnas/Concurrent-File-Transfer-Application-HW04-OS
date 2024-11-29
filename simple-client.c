#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(const char *msg) {
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[]) {
    

    //create a socket 
    int network_socket = socket(AF_INET, SOCK_STREAM, 0);

    //specify an address for the socket
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8080);
    server_address.sin_addr.s_addr = INADDR_ANY;// shortcut for IP address: 0.0.0.0 

    int connection_status = connect(network_socket, (struct sockaddr*)&server_address, sizeof(server_address));

    // check for connection
    if (connection_status == -1) {
        error("There was an error making a connection to the remote socket\n");
    }

    // receive data from the server

    char server_response[256];
    //recv = receive
    recv(network_socket, &server_response, sizeof(server_response), 0);

    //print out the server's response
    printf("The server sent the data: %s\n", server_response);


    //close the socket
    close(network_socket);
    return 0;
}
