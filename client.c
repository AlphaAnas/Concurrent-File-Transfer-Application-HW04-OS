#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

// Thread function to receive file segments
void *receive_segment(void *arg) {
    // Code for receiving a file segment
    return NULL;
}

int main(int argc, char *argv[]) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char *filename;
    int thread_count;

    if (argc != 3) {
        printf("Usage: %s <filename> <thread_count>\n", argv[0]);
        return -1;
    }

    filename = argv[1];
    thread_count = atoi(argv[2]);

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    // Connect to server
    connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    printf("Connected to server.\n");

    // Send file request
    // Receive and reassemble file

    close(sock);
    return 0;
}
