#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#define KB_SIZE 1024    // Size of each chunk to send
#define BACKLOG 5       // Max number of queued connections
#define MAX_CLIENTS 5   // Max number of simultaneous clients

// Structure to hold client and file data
struct client_data_t {
    int client_socket;
    char *file_buffer;
    long file_size;
};

// Function prototypes
void *handle_client(void *arg);
void load_file_into_memory(const char *filename, char **buffer, long *file_size);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *filename = argv[1];
    char *file_buffer = NULL;
    long file_size = 0;

    // Load the file into memory
    load_file_into_memory(filename, &file_buffer, &file_size);

    // Set up the server socket
    int server_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_len = sizeof(client_address);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8080); // Port number
    server_address.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Binding failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_socket, BACKLOG) < 0) {
        perror("Listening failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port 8080...\n");

    pthread_t threads[MAX_CLIENTS];
    int active_clients = 0;

    // Accept and handle multiple clients
    while (1) {
        int client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_len);
        if (client_socket < 0) {
            perror("Connection acceptance failed");
            continue;
        }

        printf("Client connected: %s\n", inet_ntoa(client_address.sin_addr));

        if (active_clients >= MAX_CLIENTS) {
            fprintf(stderr, "Maximum clients reached. Closing connection.\n");
            close(client_socket);
            continue;
        }

        struct client_data_t *client_data = malloc(sizeof(struct client_data_t));
        client_data->client_socket = client_socket;
        client_data->file_buffer = file_buffer;
        client_data->file_size = file_size;

        pthread_create(&threads[active_clients], NULL, handle_client, client_data);
        active_clients++;

        // Join finished threads
        for (int i = 0; i < active_clients; i++) {
            if (pthread_join(threads[i], NULL) == 0) {
                active_clients--;
            }
        }
    }

    // Clean up resources
    close(server_socket);
    free(file_buffer);

    return 0;
}

// Function to load the entire file into memory
void load_file_into_memory(const char *filename, char **buffer, long *file_size) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    fseek(file, 0L, SEEK_END);
    *file_size = ftell(file);
    rewind(file);

    *buffer = malloc(*file_size);
    if (!*buffer) {
        perror("Memory allocation failed");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    if (fread(*buffer, 1, *file_size, file) != *file_size) {
        perror("File reading failed");
        fclose(file);
        free(*buffer);
        exit(EXIT_FAILURE);
    }

    fclose(file);
}

// Function to handle a single client connection
void *handle_client(void *arg) {
    struct client_data_t *client_data = (struct client_data_t *)arg;
    int client_socket = client_data->client_socket;
    char *file_buffer = client_data->file_buffer;
    long file_size = client_data->file_size;

    // Send the file size to the client
    if (send(client_socket, &file_size, sizeof(file_size), 0) < 0) {
        perror("Failed to send file size");
        close(client_socket);
        free(client_data);
        pthread_exit(NULL);
    }

    // Send the file in chunks
    long bytes_sent = 0;
    while (bytes_sent < file_size) {
        long bytes_to_send = (file_size - bytes_sent) < KB_SIZE ? (file_size - bytes_sent) : KB_SIZE;
        if (send(client_socket, file_buffer + bytes_sent, bytes_to_send, 0) < 0) {
            perror("Error sending file data");
            break;
        }
        bytes_sent += bytes_to_send;
    }

    printf("File sent to client. Total bytes sent: %ld\n", bytes_sent);

    close(client_socket);
    free(client_data);
    pthread_exit(NULL);
}

