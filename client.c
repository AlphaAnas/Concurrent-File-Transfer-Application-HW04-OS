#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <pthread.h>


typedef struct {
    int server_socket;
    FILE *file;
    int index;
} ThreadData;

void recieve_segment_data(void *arg){
    
    ThreadData *data = (ThreadData *)arg;
    char buffer[1024];
    FILE *temp_file; // temp_file is the file where the segment data will be stored
    char temp_name[32];
    sprintf(temp_name, "segment_%d.tmp", data->index);

    temp_file = fopen(temp_name, "wb");
    if (!temp_file) error("Error opening temp file");

    int bytes_received;
    bytes_received = recv(data->server_socket, buffer, 1024, 0);
    while (bytes_received > 0) {
        fwrite(buffer, 1, bytes_received, temp_file);
        bytes_received = recv(data->server_socket, buffer, 1024, 0);
    }
    

    fclose(temp_file);
    close(data->server_socket);
    return NULL;
}

int main(int argc, char *argv[]) {

    if (argc < 5) {
        fprintf(stderr, "Usage: %s <hostname> <port> <file_name> <num_threads>\n", argv[0]);
        exit(1);
    }

    int port_no = atoi(argv[2]);
    int num_threads = atoi(argv[4]);
    char *file_name = argv[3];

    struct hostent *server = gethostbyname(argv[1]); // get the IP address of the server
    if (server == NULL) {
        fprintf(stderr, "Error, no such host\n");
        exit(0);
    }


    struct sockaddr_in server_address;
    for (int i=0; i < num_threads; i++){
        //create a socket 
        int network_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (network_socket < 0) {
            error("Error opening socket");
        }
        //specify an address for the socket
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(port_no);
        // server_address.sin_addr.s_addr = INADDR_ANY;// shortcut for IP address: 0.0.0.0 
        // bcopy copies server->h_addr to server_address.sin_addr.s_addr

        bcopy((char *)server->h_addr_list[0], (char *)&server_address.sin_addr.s_addr, server->h_length);

        // connect to the server
        int connection_status = connect(network_socket, (struct sockaddr*)&server_address, sizeof(server_address));
         // check for connection
        if (connection_status == -1) {
            error("There was an error making a connection to the remote socket\n");
        }

        printf("Connected to the server.\n");

        send(network_socket, file_name, strlen(file_name) + 1, 0);// send the file name to the server
        send(network_socket, &num_threads, sizeof(num_threads), 0); // send the number of threads to the server

        ThreadData *data = malloc(sizeof(ThreadData));
        data->server_socket = network_socket;
        data->index = i;

        pthread_t thread;
        pthread_create(&thread, NULL, recieve_segment_data, data);
        pthread_detach(thread);
    }

    // now since data is received we need to reassemble all the threads to make a file
      // Reassemble segments after download
    FILE *output_file = fopen("output_file", "wb");
    for (int i = 0; i < num_threads; i++) {
        char temp_name[32];
        sprintf(temp_name, "segment_%d.tmp", i);

        FILE *temp_file = fopen(temp_name, "rb");
        char buffer[1024];
        int bytes_read;
        bytes_read = fread(buffer, 1, 1024, temp_file);
        while (bytes_read > 0) {
            fwrite(buffer, 1, bytes_read, output_file);
            bytes_read = fread(buffer, 1, 1024, temp_file);
        }

        fclose(temp_file);
        remove(temp_name); // Delete temp file
    }

    fclose(output_file);
    printf("File transfer complete.\n");
    return 0;
}
