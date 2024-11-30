#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <openssl/md5.h>

#define SIZE 1024

// Structure to hold information about the file and the socket
struct file_segment_t {
    int sockfd;
    char filename[SIZE];
    int start_pos;
    int end_pos;
};

// Function to compute the MD5 checksum of a file
void compute_md5(char *filename, unsigned char *checksum) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Error opening file for checksum");
        return;
    }

    MD5_CTX md5_ctx;
    MD5_Init(&md5_ctx);

    char data[SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(data, 1, SIZE, file)) > 0) {
        MD5_Update(&md5_ctx, data, bytes_read);
    }

    MD5_Final(checksum, &md5_ctx);
    fclose(file);
}

// Function to handle file segment transfer
void *send_segment(void *arg) {
    struct file_segment_t *segment = (struct file_segment_t *)arg;
    FILE *file = fopen(segment->filename, "rb");
    if (!file) {
        perror("Error opening file");
        return NULL;
    }

    fseek(file, segment->start_pos, SEEK_SET);

    char buffer[SIZE];
    int bytes_to_send = segment->end_pos - segment->start_pos;
    while (bytes_to_send > 0) {
        int bytes_read = fread(buffer, 1, SIZE, file);
        if (bytes_read <= 0) {
            break;
        }

        send(segment->sockfd, buffer, bytes_read, 0);
        bytes_to_send -= bytes_read;
    }

    fclose(file);
    return NULL;
}

// Server setup and listening
int main() {
    char *ip = "127.0.0.1";
    int port = 8080;
    int e;
    int sockfd, new_sock;
    struct sockaddr_in server_addr, new_addr;
    socklen_t addr_size;
    char filename[SIZE];
    int num_threads;

    // Set up socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("[-]Error in socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = port;
    server_addr.sin_addr.s_addr = inet_addr(ip);

    e = bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (e < 0) {
        perror("[-]Error in bind");
        exit(1);
    }

    if (listen(sockfd, 10) == 0) {
        printf("[+]Server listening...\n");
    } else {
        perror("[-]Error in listening");
        exit(1);
    }

    addr_size = sizeof(new_addr);
    new_sock = accept(sockfd, (struct sockaddr*)&new_addr, &addr_size);

    // Receive the filename and number of threads
    recv(new_sock, filename, SIZE, 0);
    recv(new_sock, &num_threads, sizeof(num_threads), 0);

    printf("Received file: %s, with %d threads\n", filename, num_threads);

    // Calculate file size and segment size
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Error opening file");
        return 0;
    }

    fseek(file, 0, SEEK_END);
    int file_size = ftell(file);
    fclose(file);

    int segment_size = file_size / num_threads;
    pthread_t threads[num_threads];
    struct file_segment_t segments[num_threads];

    for (int i = 0; i < num_threads; i++) {
        segments[i].sockfd = new_sock;
        strcpy(segments[i].filename, filename);
        segments[i].start_pos = i * segment_size;
        segments[i].end_pos = (i == num_threads - 1) ? file_size : (i + 1) * segment_size;

        pthread_create(&threads[i], NULL, send_segment, &segments[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // Optionally, compute the checksum for the file
    unsigned char checksum[MD5_DIGEST_LENGTH];
    compute_md5(filename, checksum);
    printf("File checksum computed: ");
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        printf("%02x", checksum[i]);
    }
    printf("\n");

    close(new_sock);
    close(sockfd);
    return 0;
}


// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <sys/types.h> 
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <pthread.h>


// typedef struct {
//     int client_socket;
//     FILE *file;
//     long start;
//     long end;
// } ThreadData;


// void* send_segment_data(void *arg){

//     ThreadData *data = (ThreadData*) arg;

//     fseek(data->file, data->start, SEEK_SET); // set the file pointer to the start of the segment

//     long remaining_bytes = data->end - data->start;
//     char buffer[1024]; // read 1 KB at a time
//     int buffer_size = sizeof(buffer);


//     while (remaining_bytes > 0){
//         size_t to_read;
//         if (remaining_bytes > buffer_size ){
//              to_read = buffer_size;
//         }
//         else{
//              to_read = remaining_bytes;
//         }
//         fread(buffer, 1, to_read, data->file);
//         send(data->client_socket, buffer, sizeof(buffer), 0);
//         remaining_bytes -= sizeof(buffer);
//         // empty the buffer
//         memset(buffer, 0, sizeof(buffer)); //Set N bytes of S to C. memset(S,C,N)

//     }

//     fclose(data->file);
//     free(data);
    
  



         
//         // send(client_socket, server_message, sizeof(server_message), 0);
    
// }

// int main(int argc, char *argv[]) {
    
    
    
//     // if (argc < 2) {
//     //     fprintf(stderr, "Usage: %s <port>\n", argv[0]);
//     //     return 1;
//     // }

//     int server_socket, port_no;

//     // port_no = atoi(argv[1]);
//     struct sockaddr_in client_address;

//     char server_message[256] = "You have reached the server!";
//     //create a socket
//     server_socket = socket(AF_INET, SOCK_STREAM, 0);
//     if (server_socket < 0) {
//         perror("Error opening socket");
//     }

//     //specify an address for the socket
//     struct sockaddr_in server_address;
//     server_address.sin_family = AF_INET;
//     server_address.sin_port = htons(8080);
//     server_address.sin_addr.s_addr = INADDR_ANY;// shortcut for IP address:

//     //bind the socket to our specified IP and port
//     bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address));

//     listen(server_socket, 5); // 5 is the number of connections that can be waiting while the server is busy
//     printf("Server is listening on port 8080...\n");

//     while (1) {
//         socklen_t client_len = sizeof(client_address);
//         int client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_len);
//         if (client_socket < 0) {
//             perror("Error accepting connection");
//         }


//             // receive the signal from client of the name of the file and no of threads to break into
//         printf("Client connected.\n");
//         char file_name[256];
//         int num_threads=0;
//         int temp = num_threads * sizeof(int);
//         recv(client_socket, file_name, sizeof(file_name), 0); // receive the file name from the client
//         recv(client_socket, &num_threads, sizeof(num_threads), 0); // receive the number of threads from the client
//         printf("File name: %s\n", file_name);
//         printf("Number of threads: %d\n", num_threads);
//         FILE *file = fopen(file_name, "rb");
//         if (!file) {
//             perror("File not found");
//             close(client_socket);
//             continue;
//         }

        

//         //getting the length of the file (https://stackoverflow.com/questions/238603/how-can-i-get-a-files-size-in-c)
//         fseek(file, 0L, SEEK_END); // OL is long int
//         long int size = ftell(file);
//         rewind(file); // go back to the beginning of the file
//         long int segment_size = size / num_threads;

//         for (int i = 0; i < num_threads; i++) {
//             ThreadData *data = malloc(sizeof(ThreadData));
//             data->client_socket = client_socket;
//             data->file = file;
//             data->start = i * segment_size;
//             data->end = (i == num_threads - 1) ? size : (i + 1) * segment_size; // if it is the last segment, then the end is the size of the file

//             pthread_t thread;
//             pthread_create(&thread, NULL, send_segment_data, data);
//             pthread_detach(thread);
//         }


        
//         // send(client_socket, server_message, sizeof(server_message), 0);

//         // Close the client socket after sending the message
//         close(client_socket);
//         fclose(file);
//     }

    
//     close(server_socket);
//     return 0;
// }