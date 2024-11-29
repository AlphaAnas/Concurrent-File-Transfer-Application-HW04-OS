#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>


typedef struct {
    int client_socket;
    FILE *file;
    long start;
    long end;
} ThreadData;


void* send_segment_data(void *arg){

    ThreadData *data = (ThreadData*) arg;

    fseek(data->file, data->start, SEEK_SET); // set the file pointer to the start of the segment

    long remaining_bytes = data->end - data->start;
    char buffer[1024]; // read 1 KB at a time
    int buffer_size = sizeof(buffer);


    while (remaining_bytes > 0){
        size_t to_read;
        if (remaining_bytes > buffer_size ){
             to_read = buffer_size;
        }
        else{
             to_read = remaining_bytes;
        }
        fread(buffer, 1, to_read, data->file);
        send(data->client_socket, buffer, sizeof(buffer), 0);
        remaining_bytes -= sizeof(buffer);
        // empty the buffer
        memset(buffer, 0, sizeof(buffer)); //Set N bytes of S to C. memset(S,C,N)

    }

    fclose(data->file);
    free(data);
    
  



         
        // send(client_socket, server_message, sizeof(server_message), 0);
    
}

int main(int argc, char *argv[]) {
    
    
    
    // if (argc < 2) {
    //     fprintf(stderr, "Usage: %s <port>\n", argv[0]);
    //     return 1;
    // }

    int server_socket, port_no;

    // port_no = atoi(argv[1]);
    struct sockaddr_in client_address;

    char server_message[256] = "You have reached the server!";
    //create a socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error opening socket");
    }

    //specify an address for the socket
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8080);
    server_address.sin_addr.s_addr = INADDR_ANY;// shortcut for IP address:

    //bind the socket to our specified IP and port
    bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address));

    listen(server_socket, 5); // 5 is the number of connections that can be waiting while the server is busy
    printf("Server is listening on port 8080...\n");

    while (1) {
        socklen_t client_len = sizeof(client_address);
        int client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_len);
        if (client_socket < 0) {
            perror("Error accepting connection");
        }


            // receive the signal from client of the name of the file and no of threads to break into
        printf("Client connected.\n");
        char file_name[256];
        int num_threads=0;
        int temp = num_threads * sizeof(int);
        recv(client_socket, file_name, sizeof(file_name), 0); // receive the file name from the client
        recv(client_socket, &num_threads, sizeof(num_threads), 0); // receive the number of threads from the client
        printf("File name: %s\n", file_name);
        printf("Number of threads: %d\n", num_threads);
        FILE *file = fopen(file_name, "rb");
        if (!file) {
            perror("File not found");
            close(client_socket);
            continue;
        }

        

        //getting the length of the file (https://stackoverflow.com/questions/238603/how-can-i-get-a-files-size-in-c)
        fseek(file, 0L, SEEK_END); // OL is long int
        long int size = ftell(file);
        rewind(file); // go back to the beginning of the file
        long int segment_size = size / num_threads;

        for (int i = 0; i < num_threads; i++) {
            ThreadData *data = malloc(sizeof(ThreadData));
            data->client_socket = client_socket;
            data->file = file;
            data->start = i * segment_size;
            data->end = (i == num_threads - 1) ? size : (i + 1) * segment_size; // if it is the last segment, then the end is the size of the file

            pthread_t thread;
            pthread_create(&thread, NULL, send_segment_data, data);
            pthread_detach(thread);
        }


        
        // send(client_socket, server_message, sizeof(server_message), 0);

        // Close the client socket after sending the message
        close(client_socket);
        fclose(file);
    }

    
    close(server_socket);
    return 0;
}