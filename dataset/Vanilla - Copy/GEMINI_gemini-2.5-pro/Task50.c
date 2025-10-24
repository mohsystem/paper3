#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>

// Note: This C code is specific to POSIX-compliant systems (like Linux, macOS)
// It will not compile on Windows without modification (e.g., using Winsock).

#define PORT 8083
#define UPLOAD_DIR "uploads_c"
#define BUFFER_SIZE 4096

volatile int server_running = 1;

void* handle_connection(void* client_socket_ptr) {
    int client_socket = *(int*)client_socket_ptr;
    free(client_socket_ptr);

    // 1. Read file name length
    uint32_t name_len_net;
    if (read(client_socket, &name_len_net, sizeof(name_len_net)) != sizeof(name_len_net)) {
        perror("Failed to read name length");
        close(client_socket);
        return NULL;
    }
    uint32_t name_len = ntohl(name_len_net);

    // 2. Read file name
    char* name_buffer = (char*)malloc(name_len + 1);
    if (read(client_socket, name_buffer, name_len) != name_len) {
        perror("Failed to read name");
        free(name_buffer);
        close(client_socket);
        return NULL;
    }
    name_buffer[name_len] = '\0';

    // 3. Read file content length
    uint64_t content_len_net;
    if (read(client_socket, &content_len_net, sizeof(content_len_net)) != sizeof(content_len_net)) {
        perror("Failed to read content length");
        free(name_buffer);
        close(client_socket);
        return NULL;
    }
    // htonll is not standard, so we assume same endianness for this example
    uint64_t content_len = content_len_net; 
    
    // 4. Read content and write to file
    char full_path[256];
    snprintf(full_path, sizeof(full_path), "%s/%s", UPLOAD_DIR, name_buffer);
    FILE* fp = fopen(full_path, "wb");
    if (fp == NULL) {
        perror("Failed to open file for writing");
        free(name_buffer);
        close(client_socket);
        return NULL;
    }

    char content_buffer[BUFFER_SIZE];
    uint64_t bytes_received = 0;
    while (bytes_received < content_len) {
        ssize_t to_read = (content_len - bytes_received < BUFFER_SIZE) ? (content_len - bytes_received) : BUFFER_SIZE;
        ssize_t bytes_read = read(client_socket, content_buffer, to_read);
        if (bytes_read <= 0) {
            perror("Connection closed while receiving file content");
            break;
        }
        fwrite(content_buffer, 1, bytes_read, fp);
        bytes_received += bytes_read;
    }
    fclose(fp);
    printf("Server received and saved file: %s\n", full_path);

    // 5. Send confirmation
    char response[512];
    snprintf(response, sizeof(response), "File uploaded successfully to %s", full_path);
    write(client_socket, response, strlen(response));
    
    free(name_buffer);
    close(client_socket);
    return NULL;
}

void* start_server(void* arg) {
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        return NULL;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        close(server_fd);
        return NULL;
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        return NULL;
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen");
        close(server_fd);
        return NULL;
    }
    
    printf("C server started on port %d\n", PORT);

    while (server_running) {
        int client_socket = accept(server_fd, NULL, NULL);
        if (client_socket < 0) {
            if (server_running) perror("accept");
            continue;
        }
        
        pthread_t tid;
        int *p_client_socket = malloc(sizeof(int));
        *p_client_socket = client_socket;
        pthread_create(&tid, NULL, handle_connection, p_client_socket);
        pthread_detach(tid);
    }
    
    close(server_fd);
    printf("C server stopped.\n");
    return NULL;
}


char* upload_file(const char* host, int port, const char* file_name, const char* file_content, size_t content_len) {
    int sock;
    struct sockaddr_in serv_addr;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return strdup("Client Error: Socket creation error");
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    inet_pton(AF_INET, host, &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        return strdup("Client Error: Connection Failed");
    }

    // 1. Send file name length
    uint32_t name_len = strlen(file_name);
    uint32_t name_len_net = htonl(name_len);
    send(sock, &name_len_net, sizeof(name_len_net), 0);

    // 2. Send file name
    send(sock, file_name, name_len, 0);

    // 3. Send file content length
    uint64_t content_len_64 = content_len;
    send(sock, &content_len_64, sizeof(content_len_64), 0); // Assuming same endianness

    // 4. Send file content
    send(sock, file_content, content_len, 0);

    // 5. Receive response
    char* response_buffer = (char*)malloc(1024);
    memset(response_buffer, 0, 1024);
    read(sock, response_buffer, 1023);
    close(sock);
    
    char* final_response = (char*)malloc(1050);
    snprintf(final_response, 1050, "Client Response: %s", response_buffer);
    free(response_buffer);
    return final_response;
}

int main() {
    mkdir(UPLOAD_DIR, 0755);

    // 1. Start server
    pthread_t server_thread_id;
    pthread_create(&server_thread_id, NULL, start_server, NULL);
    
    sleep(1); // Give server a moment to start

    // 2. Run test cases
    printf("\n--- Running C Test Cases ---\n");
    for (int i = 1; i <= 5; ++i) {
        char file_name[32];
        char content[64];
        snprintf(file_name, sizeof(file_name), "test%d.txt", i);
        snprintf(content, sizeof(content), "This is C test file %d.", i);
        printf("Test %d: Uploading '%s'...\n", i, file_name);
        char* result = upload_file("127.0.0.1", PORT, file_name, content, strlen(content));
        printf("%s\n", result);
        free(result);
    }
    printf("--- C Test Cases Finished ---\n\n");
    
    // 3. Stop server
    server_running = 0;
    // Make a dummy connection to unblock accept()
    int dummy_sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    connect(dummy_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    close(dummy_sock);

    pthread_join(server_thread_id, NULL);

    return 0;
}