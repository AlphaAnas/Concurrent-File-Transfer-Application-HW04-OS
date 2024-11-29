#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/socket.h>



void write_file(int sockfd){
  int n;
  FILE *fp;
  char *filename = "recv.txt";
  char buffer[1024];

  fp = fopen(filename, "w");
  while (1) {
    n = recv(sockfd, buffer, 1024, 0);
    if (n <= 0){
      break;
      return;
    }
    fprintf(fp, "%s", buffer);
    bzero(buffer, 1024); // clear the buffer
  }
  return;
}

int main(int argc, char *argv[]){



  char *ip = "127.0.0.1";
  int port = 8080;
  int binded;
  char server_message[256] = "You have reached the server!";
  printf( "Server message: %s\n", server_message);
  printf("Server is listening on port 8080...\n");
  int server_socket, new_sock;
  struct sockaddr_in server_address, new_addr;
  socklen_t address_size;
  char buffer[1024];

  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if(server_socket < 0) {
    perror("Error opening socket");
    exit(1);
  }
  printf(" Coneected to Server socket  successfully.\n");

  server_address.sin_family = AF_INET;
  server_address.sin_port = port;
  server_address.sin_addr.s_addr = inet_addr(ip);


    //bind the socket to our specified IP and port
  binded = bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address));
  if(binded < 0) {
    perror("Error in binding to the PORT");
    exit(1);
  }
  printf("Binding successfully done.\n");

  if(listen(server_socket, 10) == 0){// 10 is the number of connections that can be waiting while the server is busy
		printf(" Listening....\n");
	}else{
		perror("Error in listening");
    exit(1);
	}

  address_size = sizeof(address_size);
  new_sock = accept(server_socket, (struct sockaddr*)&new_addr, &address_size);
  write_file(new_sock);
  printf("Data written in the file successfully.\n");

  return 0;
}