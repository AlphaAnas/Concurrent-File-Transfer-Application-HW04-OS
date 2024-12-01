
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <openssl/sha.h> 

#define KB_SIZE 1024    // Chunk to send the file into 
#define BACKLOG 10      // Max queued connections - more detail when i am using it
#define MAX_FILENAME 256

struct client_data_t {
    int client_socket;
};

void *handle_client(void *arg);
void compute_sha256(const char *filename, unsigned char *hash_output);

int main() {

       //specify an address for the socket
    int server_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_len = sizeof(client_address);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8080);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Binding failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, BACKLOG) < 0) { // BACKLOG is the number of connections that can be waiting while the server is busy
        perror("Listening failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port 8080...\n");

    while (1) {
        int client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_len);
        if (client_socket < 0) {
            perror("Connection acceptance failed");
            continue;
        }

        printf("Client connected: %s\n", inet_ntoa(client_address.sin_addr));

        pthread_t thread;
        struct client_data_t *client_data = malloc(sizeof(struct client_data_t));
        client_data->client_socket = client_socket;

        pthread_create(&thread, NULL, handle_client, client_data);
        pthread_detach(thread);  
    }

    close(server_socket);
    return 0;
}

// Function to compute SHA256 hash of a file
void compute_sha256(const char *filename, unsigned char *hash_output) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Error opening file for hash computation");
        return;
    }

    SHA256_CTX sha256;
    SHA256_Init(&sha256);

    unsigned char buffer[KB_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, KB_SIZE, file)) > 0) {
        SHA256_Update(&sha256, buffer, bytes_read);
    }

    SHA256_Final(hash_output, &sha256);
    fclose(file);
}

void *handle_client(void *arg) {
    struct client_data_t *client_data = (struct client_data_t *)arg;
    int client_socket = client_data->client_socket;
    free(client_data);

    char filename[MAX_FILENAME];
    memset(filename, 0, MAX_FILENAME); //Set N bytes of S to C. memset(S,C,N)

    // Receive the filename from the client
    if (recv(client_socket, filename, MAX_FILENAME, 0) <= 0) {
        perror("Error receiving filename");
        close(client_socket);
        pthread_exit(NULL);
    }

    printf("Client requested file: %s\n", filename);

    // Load the file into memory
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Error opening file");
        long error = -1;
        send(client_socket, &error, sizeof(error), 0);
        close(client_socket);
        pthread_exit(NULL);
    }

       //getting the length of the file (https://stackoverflow.com/questions/238603/how-can-i-get-a-files-size-in-c)
    fseek(file, 0, SEEK_END); // set the file pointer to the start of the segment
    long file_size = ftell(file); // go back to the beginning of the file
    rewind(file);

    // Allocate memory for file
    char *file_buffer = malloc(file_size);
    if (!file_buffer) {
        perror("Memory allocation failed");
        fclose(file);
        close(client_socket);
        pthread_exit(NULL);
    }

    fread(file_buffer, 1, file_size, file);
    fclose(file);

        // Before sending the file size, compute the hash
    unsigned char file_hash[SHA256_DIGEST_LENGTH];
    compute_sha256(filename, file_hash);

    // Send the hash to the client
    if (send(client_socket, file_hash, SHA256_DIGEST_LENGTH, 0) < 0) {
        perror("Failed to send file hash");
        close(client_socket);
        pthread_exit(NULL);
    }


    // Send file size to client
    if (send(client_socket, &file_size, sizeof(file_size), 0) < 0) {
        perror("Failed to send file size");
        free(file_buffer);
        close(client_socket);
        pthread_exit(NULL);
    }

    // Send file content to client
    long bytes_sent = 0;
    while (bytes_sent < file_size) {
        long chunk_size = (file_size - bytes_sent) < KB_SIZE ? (file_size - bytes_sent) : KB_SIZE; // if the remaining bytes are less than 1024, then send the remaining bytes
        if (send(client_socket, file_buffer + bytes_sent, chunk_size, 0) < 0) {
            perror("Error sending file data");
            break;
        }
        bytes_sent += chunk_size;
    }

    printf("File sent to client. Total bytes sent: %ld\n", bytes_sent);

    free(file_buffer);
    close(client_socket);
    pthread_exit(NULL);
}
