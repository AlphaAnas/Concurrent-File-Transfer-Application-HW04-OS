#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>





void send_segment_data(FILE *fp, int sockfd){
  int n;
  char buffer[1024];

  while(fgets(buffer, 1024, fp) != NULL) {
    if (send(sockfd, buffer, sizeof(buffer), 0) == -1) {
      perror("Error in sending file.");
      exit(1);
    }
    bzero(buffer, 1024);
  }
}

int main(int argc, char *argv[]) {

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <file_name> <num_threads>\n", argv[0]);
        exit(1);
    }

  char* file_name = argv[1];
  int num_threads = atoi(argv[2]);

    if (num_threads <= 0) {
        fprintf(stderr, "Error: Number of threads must be positive.\n");
        exit(1);
    }
  char *ip = "127.0.0.1";  // the default ip address of the server I saw it at : https://www.geeksforgeeks.org/socket-programming-cc/
  int port = 8080;
  int connection_status;

  int sockfd;
  struct sockaddr_in server_address;
  FILE *file_ptr;
//   char *filename = "send.txt";
      //create a socket for the client
  int network_socket = socket(AF_INET, SOCK_STREAM, 0);
  if(network_socket < 0) {
    perror("There is an Error in socket");
    exit(1);
  }
  printf("Congo! Server socket created successfully.\n");

   //specify an address for the socket
  server_address.sin_family = AF_INET;
  server_address.sin_port = port;
  server_address.sin_addr.s_addr = inet_addr(ip);

  connection_status = connect(network_socket, (struct sockaddr*)&server_address, sizeof(server_address));
  if(connection_status == -1) {
    perror("Error in connecting to socket");
    exit(1);
  }
	printf("Connected to Server.\n");

  file_ptr = fopen(file_name, "r");
  if (file_ptr == NULL) {
    perror("Error in reading file.");
    exit(1);
  }

  send_segment_data(file_ptr, network_socket);
  printf(" File data sent successfully.\n");

	printf("Closing the connection.\n");
  close(network_socket);

  return 0;
}