#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>

struct filedata_t {
  char filename[1024];
  int new_sock;
  int s_index;
  int e_index;
  int num_threads;
  int e_index_total; // Total end index (file size) for full reassembly
} filedata;


void *receive_file(void *arg) {
    struct filedata_t *filedata = (struct filedata_t *)arg;
    int bytes_received;
    FILE *fp;
    char *buffer;
    
    // Dynamically allocate memory for the buffer
    buffer = (char *)malloc(1024 * sizeof(char));
    if (buffer == NULL) {
        perror("Error allocating memory for buffer");
        return NULL;
    }

    // Open the file for writing (create the file if it doesn't exist)
    fp = fopen("received_file.txt", "ab");  // "ab" mode to append data to the file
    if (fp == NULL) {
        perror("Error opening file for receiving");
        free(buffer);
        return NULL;
    }
    printf("created file named received_file.txt \n");

    int bytes_to_receive = filedata->e_index - filedata->s_index;

    // Seek to the correct position where the segment should be written
    fseek(fp, filedata->s_index, SEEK_SET);

    while (bytes_to_receive > 0) {
        bytes_received = recv(filedata->new_sock, buffer, 1024, 0);  // Receive data into buffer

        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                printf("[+]Connection closed by the server.\n");
            } else {
                perror("[-]Error in receiving data");
            }
            break;
        }

        // Write the received data to the file at the correct position
        fwrite(buffer, 1, bytes_received, fp);

        // Decrease the remaining bytes to receive
        bytes_to_receive -= bytes_received;
    }

    // Close the file and free the buffer memory
    fclose(fp);
    free(buffer);

    return NULL;
}


int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <file_name> <num_threads>\n", argv[0]);
        exit(1);
    }

    // save file name and number of threads
    strncpy(filedata.filename, argv[1], sizeof(filedata.filename));
    filedata.num_threads = atoi(argv[2]);
    filedata.e_index_total = 1024  * 490;  // Example total file size (490KB)

    if (filedata.num_threads <= 0) {
        fprintf(stderr, "Error: Number of threads must be positive.\n");
        exit(1);
    }

    char *ip = "127.0.0.1";  // Default IP address
    int port = 8080;
    int connection_status;

    // Create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error in creating socket");
        exit(1);
    }

    struct sockaddr_in server_address;

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = inet_addr(ip);

    // Connect to the server
    connection_status = connect(sockfd, (struct sockaddr*)&server_address, sizeof(server_address));
    if (connection_status == -1) {
        perror("Error in connecting to socket");
        exit(1);
    }

    printf("Connected to Server.\n");

    // Send file and thread request to the server
    send(sockfd, &filedata, sizeof(filedata), 0);


    // Create threads for receiving file segments dynamically
    pthread_t threads[filedata.num_threads];
    struct filedata_t segments[filedata.num_threads];

    // Calculate dynamic segment sizes and indexes
    int segment_size = filedata.e_index_total / filedata.num_threads;
    for (int i = 0; i < filedata.num_threads; i++) {
        segments[i].new_sock = sockfd;
        strcpy(segments[i].filename, filedata.filename);
        // segments[i].filename = filedata.filename;
        segments[i].num_threads = filedata.num_threads;
        segments[i].e_index = (i + 1) * segment_size;
        segments[i].s_index = i * segment_size;

        // Ensure the last thread gets the remaining bytes if file size is not perfectly divisible
        if (i == filedata.num_threads - 1) {
            segments[i].e_index = filedata.e_index_total;
        }

        pthread_create(&threads[i], NULL, receive_file, &segments[i]);
    }

    for (int i = 0; i < filedata.num_threads; i++) {
        
        pthread_join(threads[i], NULL);
    }





    // Close the connection
    printf("Closing the connection.\n");
    close(sockfd);


    return 0;
}
