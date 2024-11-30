#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <openssl/md5.h>

#define SIZE 1024

struct file_segment_t {
    int sockfd;
    char filename[SIZE];
    int start_pos;
    int end_pos;
};

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

// Function to reassemble file
void *receive_segment(void *arg) {
    struct file_segment_t *segment = (struct file_segment_t *)arg;
    FILE *file = fopen(segment->filename, "r+b");
    if (!file) {
        perror("Error opening file");
        return NULL;
    }

    fseek(file, segment->start_pos, SEEK_SET);
    char buffer[SIZE];
    int bytes_to_receive = segment->end_pos - segment->start_pos;

    while (bytes_to_receive > 0) {
        int bytes_received = recv(segment->sockfd, buffer, SIZE, 0);
        if (bytes_received <= 0) {
            break;
        }
        fwrite(buffer, 1, bytes_received, file);
        bytes_to_receive -= bytes_received;
    }

    fclose(file);
    return NULL;
}

int main() {
    char *ip = "127.0.0.1";
    int port = 8080;
    int sockfd;
    struct sockaddr_in server_addr;
    char filename[SIZE];
    int num_threads;

    // Get the file name and number of threads from user input
    printf("Enter file name: ");
    scanf("%s", filename);
    printf("Enter number of threads: ");
    scanf("%d", &num_threads);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error in socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = port;
    server_addr.sin_addr.s_addr = inet_addr(ip);

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error in connecting");
        exit(1);
    }

    // Send filename and number of threads to the server
    send(sockfd, filename, strlen(filename) + 1, 0);
    send(sockfd, &num_threads, sizeof(num_threads), 0);

    pthread_t threads[num_threads];
    struct file_segment_t segments[num_threads];

    // Create threads to receive file segments
    for (int i = 0; i < num_threads; i++) {
        segments[i].sockfd = sockfd;
        strcpy(segments[i].filename, filename);
        segments[i].start_pos = i * (1024 * 1024); // assuming segment size of 1MB
        segments[i].end_pos = (i + 1) * (1024 * 1024);
        pthread_create(&threads[i], NULL, receive_segment, &segments[i]);
    }

    // Join the threads
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // Compute checksum of the reassembled file
    unsigned char checksum[MD5_DIGEST_LENGTH];
    compute_md5(filename, checksum);
    printf("File checksum after reassembly: ");
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        printf("%02x", checksum[i]);
    }
    printf("\n");




// #include <stdio.h>
// #include <stdlib.h>
// #include <unistd.h>
// #include <string.h>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <netdb.h> 
// #include <pthread.h>


// typedef struct {
//     int server_socket;
//     FILE *file;
//     int index;
// } ThreadData;


// void *recieve_segment_data(void *arg){
    
//     ThreadData *data = (ThreadData *)arg;
//     char buffer[1024];
//     FILE *temp_file; // temp_file is the file where the segment data will be stored
//     char temp_name[32];
//     sprintf(temp_name, "segment_%d.tmp", data->index);

//     temp_file = fopen(temp_name, "wb");
//     if (!temp_file) perror("Error opening temp file");

//     int bytes_received;
//     bytes_received = recv(data->server_socket, buffer, 1024, 0); // receive the data from the server by copying it to the buffer
//     while (bytes_received > 0) {
//         fwrite(buffer, 1, bytes_received, temp_file);
//         bytes_received = recv(data->server_socket, buffer, 1024, 0);
//     }
    

//     fclose(temp_file);
//     close(data->server_socket);
//     // return NULL;
// }

// int main(int argc, char *argv[]) {

//     if (argc < 3) {
//             fprintf(stderr, "Usage: %s <file_name> <num_threads>\n", argv[0]);
//             exit(1);
//         }

//     char *file_name = argv[1];
//     int num_threads = atoi(argv[2]);

//     if (num_threads <= 0) {
//         fprintf(stderr, "Error: Number of threads must be positive.\n");
//         exit(1);
//     }

//     // Hardcoded hostname and port
//     const char *hostname = "localhost";
//     const int port_no = 8080;

//     struct hostent *server = gethostbyname(hostname); // get the IP address of the server
//     if (server == NULL) {
//         fprintf(stderr, "Error, no such host\n");
//         exit(0);
//     }


//     struct sockaddr_in server_address;
//     memset(&server_address, 0, sizeof(server_address));
//     server_address.sin_family = AF_INET;
//     server_address.sin_port = htons(port_no);
//     memcpy(&server_address.sin_addr.s_addr, server->h_addr_list[0], server->h_length);
//     // send your request to the server
//     for (int i=0; i < num_threads; i++){
      
//         int network_socket = socket(AF_INET, SOCK_STREAM, 0);
//         if (network_socket < 0) {
//             perror("Error opening socket");
//             exit(1);
//         }
//         //specify an address for the socket
//         server_address.sin_family = AF_INET;
//         server_address.sin_port = htons(port_no);
//         // server_address.sin_addr.s_addr = INADDR_ANY;// shortcut for IP address: 0.0.0.0 

//         // bcopy copies server->h_addr to server_address.sin_addr.s_addr

//         bcopy((char *)server->h_addr_list[0], (char *)&server_address.sin_addr.s_addr, server->h_length);

//         // connect to the server
//         int connection_status = connect(network_socket, (struct sockaddr*)&server_address, sizeof(server_address));
//          // check for connection
//         if (connection_status == -1) {
//             perror("There was an error making a connection to the remote socket\n");
//         }

//         printf("Connected to the server.\n");
//         printf("Sending %d threads to the server\n", num_threads);
//         // all threads size
//         int temp = num_threads * sizeof(int);
//         send(network_socket, file_name, strlen(file_name) + 1, 0);// send the file name to the server to download
//         send(network_socket, &num_threads, sizeof(num_threads), 0); // send the number of threads to the server to break the file into

//         ThreadData *data = malloc(sizeof(ThreadData)); // data is the struct that will be passed to the thread
//         data->server_socket = network_socket; // the socket to the server
//         data->index = i;

//         pthread_t thread;
//         pthread_create(&thread, NULL, recieve_segment_data, data);
//         pthread_detach(thread);
//     }

//     // now since data is received we need to reassemble all the threads to make a file
//       // Reassemble segments after download
//     FILE *output_file = fopen("output_file", "wb");
//     for (int i = 0; i < num_threads; i++) {
//         char temp_name[32];
//         sprintf(temp_name, "segment_%d.tmp", i);

//         FILE *temp_file = fopen(temp_name, "rb");
//         char buffer[1024];
//         int bytes_read;
//         bytes_read = fread(buffer, 1, 1024, temp_file);
//         while (bytes_read > 0) {
//             fwrite(buffer, 1, bytes_read, output_file);
//             bytes_read = fread(buffer, 1, 1024, temp_file);
//         }

//         fclose(temp_file);
//         remove(temp_name); // Delete temp file
//     }

//     fclose(output_file);
//     printf("File transfer complete.\n");
//     return 0;
// }
