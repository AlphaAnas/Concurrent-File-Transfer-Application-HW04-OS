#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>

struct filedata_t {
  char filename[1024];
  int new_sock;
  int s_index;
  int e_index;
  int num_threads;
} filedata;

void *send_file(void *arg) {

    struct filedata_t *filedata = (struct filedata_t *)arg;
    int bytes_read;
    FILE *fp;
    char buffer[1024];

    
    // Opening file for writing
    fp = fopen(filedata->filename, "rb"); 
    if (fp == NULL ) {
        perror("Error opening fp file");
        return;
    }
    int bytes_to_read = filedata->e_index - filedata->s_index;
    while (bytes_to_read > 0) {
        bytes_read = fread(buffer, 1, 1024, fp);
        if (bytes_read <= 0) {
            break;
        }
        send(filedata->new_sock, buffer, bytes_read, 0);
        bytes_to_read -= bytes_read;
        
    }
    // Write data to file
    fclose(fp);
    return NULL;
}

int main(int argc, char *argv[]) {
    char *ip = "127.0.0.1";
    int port = 8080;
    int server_socket, new_sock;
    struct sockaddr_in server_address, new_addr;
    socklen_t address_size;
    char buffer[1024];

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error opening socket");
        exit(1);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;  // Use INADDR_ANY to bind to all interfaces

    // Bind the socket
    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("Error in binding to the PORT");
        exit(1);
    }

    // Listen for connections
    if (listen(server_socket, 10) == 0) {
        printf("Listening....\n");
    } else {
        perror("Error in listening");
        exit(1);
    }

    address_size = sizeof(new_addr);
    new_sock = accept(server_socket, (struct sockaddr*)&new_addr, &address_size);
    if (new_sock < 0) {
        perror("Error in accepting connection");
        exit(1);
    }

    printf("Connection accepted from client.\n");

    // Receive file request and number of threads
    recv(new_sock, &filedata, sizeof(filedata), 0);
    printf("Received request for file %s\n", filedata.filename);
    printf("Number of threads received: %d\n", filedata.num_threads);


    //find the start and end positions of the file
    FILE *file = fopen(filedata.filename, "r");
      //getting the length of the file (https://stackoverflow.com/questions/238603/how-can-i-get-a-files-size-in-c)
    fseek(file, 0L, SEEK_END); // OL is long int
    long int file_size = ftell(file);
    rewind(file); // go back to the beginning of the file
    long int segment_size = file_size / filedata.num_threads;

    pthread_t threads[filedata.num_threads];
    struct filedata_t segments[filedata.num_threads];

    for (int i = 0; i < filedata.num_threads; i++) {
        strcpy(segments[i].filename, filedata.filename);
        // segments[i].filename = filedata.filename;
        segments[i].num_threads = filedata.num_threads;
        segments[i].s_index = i * segment_size;
        segments[i].e_index = (i == filedata.num_threads - 1) ? file_size : (i + 1) * segment_size;
        segments[i].new_sock = new_sock;
        pthread_create(&threads[i], NULL, send_file, (void *)&segments[i]);
    }
    printf("Data written in the file received.txt successfully.\n");

    for (int i = 0; i < filedata.num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    close(new_sock);
    close(server_socket);
    return 0;
}
