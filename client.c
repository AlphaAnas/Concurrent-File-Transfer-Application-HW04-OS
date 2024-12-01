#include <openssl/sha.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

#define KB_SIZE 1024 // 1 KB
#define DEFAULT_PORT 8080 // Default port to connect to
#define MAX_FILENAME 256

int connect_to_server(const char *ip, int port);
void send_filename(int sockfd, const char *filename);
void receive_file(int sockfd, const char *output_filename, const char * old_filename);
void compute_sha256(const char *filename, unsigned char *hash_output);

int main(int argc, char *argv[]) {
    const char *server_ip = "127.0.0.1";
    int port = DEFAULT_PORT;

    int num_threads;

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <filename> <num_threads> \n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char* filename = argv[1];
    num_threads = atoi(argv[2]);
   
    int sockfd = connect_to_server(server_ip, port);

    send_filename(sockfd, filename);
    char new_filename[63];
    snprintf(new_filename, sizeof(new_filename), "out_%s", filename);



    receive_file(sockfd, new_filename, filename);  // Save file with out+filename

    close(sockfd);
    return 0;
}

int connect_to_server(const char *ip, int port) {
    int sockfd;
    struct sockaddr_in server_address;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET; // short for IPv4
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = inet_addr(ip);

    if (connect(sockfd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Connection to server failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

void send_filename(int sockfd, const char *filename) {
    if (send(sockfd, filename, strlen(filename) + 1, 0) < 0) {  // Include null terminator
        perror("Error sending filename");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
}

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

void receive_file(int sockfd, const char *output_filename, const char* old_filename) {
    char buffer[KB_SIZE];
    long file_size = 0;
    long bytes_received = 0;


        // Receive the hash from the server
    unsigned char server_hash[SHA256_DIGEST_LENGTH];
    if (recv(sockfd, server_hash, SHA256_DIGEST_LENGTH, 0) <= 0) {
        perror("Error receiving file hash");
        exit(EXIT_FAILURE);
    }

    // Compute the hash of the received file
    unsigned char local_hash[SHA256_DIGEST_LENGTH];
    compute_sha256(old_filename, local_hash);

    // Compare the hashes
    //     int memcmp(const void *__s1, const void *__s2, size_t __n)
    // Compare N bytes of S1 and S2.
    if (memcmp(server_hash, local_hash, SHA256_DIGEST_LENGTH) == 0) { // 
        printf("File integrity verified: Hash matches.\n");
    } else {
        printf("File integrity check failed: Hash mismatch.\n");
    }


    // Receive file size
    if (recv(sockfd, &file_size, sizeof(file_size), 0) <= 0) {
        perror("Error receiving file size");
        exit(EXIT_FAILURE);
    }

    if (file_size < 0) {
        printf("Server could not open the requested file.\n");
        exit(EXIT_FAILURE);
    }

    printf("Receiving file of size: %ld bytes\n", file_size);

    FILE *fp = fopen(output_filename, "wb");
    if (!fp) {
        perror("Error opening output file");
        exit(EXIT_FAILURE);
    }

    while (bytes_received < file_size) {
        int chunk_size = recv(sockfd, buffer, KB_SIZE, 0);
        if (chunk_size <= 0) {
            perror("Error receiving file data");
            break;
        }
        fwrite(buffer, 1, chunk_size, fp);
        bytes_received += chunk_size;
    }

    fclose(fp);
    printf("File received successfully: %s\n", output_filename);
}
