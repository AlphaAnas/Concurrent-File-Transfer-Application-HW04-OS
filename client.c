#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#define KB_SIZE 1024  // Buffer size for receiving data
#define DEFAULT_PORT 8080

// Function prototypes
int connect_to_server(const char *ip, int port);
void receive_file(int sockfd, const char *output_filename);

// Main Function
int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <server_ip> [port]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *server_ip = argv[1];
    int port = (argc >= 3) ? atoi(argv[2]) : DEFAULT_PORT;

    // Connect to the server
    int sockfd = connect_to_server(server_ip, port);

    // Receive the file
    receive_file(sockfd, "parents_video_out.mp4");

    // Close the connection
    close(sockfd);

    return 0;
}

// Connects to the server and returns the socket file descriptor
int connect_to_server(const char *ip, int port) {
    int sockfd;
    struct sockaddr_in server_address;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = inet_addr(ip);

    if (connect(sockfd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Connection to server failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server at %s:%d\n", ip, port);
    return sockfd;
}

// Receives a file from the server and saves it to the given output filename
void receive_file(int sockfd, const char *output_filename) {
    char buffer[KB_SIZE];
    long file_size = 0;
    long bytes_received = 0;

    // Receive the file size from the server
    if (recv(sockfd, &file_size, sizeof(file_size), 0) <= 0) {
        perror("Error receiving file size");
        exit(EXIT_FAILURE);
    }

    printf("Receiving file of size: %ld bytes\n", file_size);

    // Open the file for writing
    FILE *fp = fopen(output_filename, "wb");
    if (!fp) {
        perror("Error opening output file");
        exit(EXIT_FAILURE);
    }

    // Receive the file data in chunks
    while (bytes_received < file_size) {
        int chunk_size = recv(sockfd, buffer, KB_SIZE, 0);
        if (chunk_size < 0) {
            perror("Error receiving file data");
            fclose(fp);
            exit(EXIT_FAILURE);
        } else if (chunk_size == 0) {
            printf("Connection closed by server\n");
            break;
        }

        fwrite(buffer, 1, chunk_size, fp);
        bytes_received += chunk_size;

        printf("Progress: %ld/%ld bytes received\n", bytes_received, file_size);
    }

    fclose(fp);

    if (bytes_received == file_size) {
        printf("File received successfully and saved as %s\n", output_filename);
    } else {
        fprintf(stderr, "Warning: Incomplete file received (%ld/%ld bytes)\n", bytes_received, file_size);
    }
}

