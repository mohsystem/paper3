
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8888
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 100

typedef struct {
    int socket;
    char name[50];
    int active;
} Client;

typedef struct {
    Client clients[MAX_CLIENTS];
    int client_count;
    pthread_mutex_t mutex;
} ServerData;

ServerData server_data = {
    .client_count = 0,
    .mutex = PTHREAD_MUTEX_INITIALIZER
};

void broadcast_message(const char* message, int sender_socket) {
    printf("%s\\n", message);
    pthread_mutex_lock(&server_data.mutex);
    
    for (int i = 0; i < server_data.client_count; i++) {
        if (server_data.clients[i].active) {
            char msg[BUFFER_SIZE + 1];
            snprintf(msg, sizeof(msg), "%s\\n", message);
            send(server_data.clients[i].socket, msg, strlen(msg), 0);
        }
    }
    
    pthread_mutex_unlock(&server_data.mutex);
}

void* handle_client(void* arg) {
    int client_socket = *(int*)arg;
    free(arg);
    
    char buffer[BUFFER_SIZE];
    char client_name[50] = "Anonymous";
    int client_index = -1;
    
    // Get client name
    const char* prompt = "Enter your name: ";
    send(client_socket, prompt, strlen(prompt), 0);
    
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\\0';
        sscanf(buffer, "%49s", client_name);
        
        if (strlen(client_name) == 0) {
            strcpy(client_name, "Anonymous");
        }
    }
    
    // Add client to list
    pthread_mutex_lock(&server_data.mutex);
    if (server_data.client_count < MAX_CLIENTS) {
        client_index = server_data.client_count;
        server_data.clients[client_index].socket = client_socket;
        strncpy(server_data.clients[client_index].name, client_name, sizeof(client_name) - 1);
        server_data.clients[client_index].active = 1;
        server_data.client_count++;
    }
    pthread_mutex_unlock(&server_data.mutex);
    
    char join_msg[BUFFER_SIZE];
    snprintf(join_msg, sizeof(join_msg), "%s has joined the chat!", client_name);
    broadcast_message(join_msg, client_socket);
    
    // Handle messages
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        
        if (bytes_received <= 0) {
            break;
        }
        
        buffer[bytes_received] = '\\0';
        
        // Remove newline characters
        char* newline = strchr(buffer, '\\n');
        if (newline) *newline = '\\0';
        newline = strchr(buffer, '\\r');
        if (newline) *newline = '\\0';
        
        if (strcmp(buffer, "/quit") == 0) {
            break;
        }
        
        char msg[BUFFER_SIZE];
        snprintf(msg, sizeof(msg), "%s: %s", client_name, buffer);
        broadcast_message(msg, client_socket);
    }
    
    // Cleanup
    pthread_mutex_lock(&server_data.mutex);
    if (client_index >= 0 && client_index < server_data.client_count) {
        server_data.clients[client_index].active = 0;
    }
    pthread_mutex_unlock(&server_data.mutex);
    
    char leave_msg[BUFFER_SIZE];
    snprintf(leave_msg, sizeof(leave_msg), "%s has left the chat!", client_name);
    broadcast_message(leave_msg, client_socket);
    
    close(client_socket);
    return NULL;
}

void start_server(int port) {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error creating socket");
        return;
    }
    
    // Set socket options
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // Bind socket
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding socket");
        close(server_socket);
        return;
    }
    
    // Listen for connections
    if (listen(server_socket, 5) < 0) {
        perror("Error listening on socket");
        close(server_socket);
        return;
    }
    
    printf("Chat server started on port %d\\n", port);
    
    // Accept connections
    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            perror("Error accepting connection");
            continue;
        }
        
        printf("New client connected\\n");
        
        // Handle client in a separate thread
        pthread_t thread;
        int* client_sock_ptr = malloc(sizeof(int));
        *client_sock_ptr = client_socket;
        pthread_create(&thread, NULL, handle_client, client_sock_ptr);
        pthread_detach(thread);
    }
    
    close(server_socket);
}

int main() {
    printf("=== Chat Server Application ===\\n");
    printf("Test Case 1: Starting server on port %d\\n", PORT);
    
    printf("\\nTo test this application:\\n");
    printf("1. Run this server\\n");
    printf("2. Use telnet or netcat to connect: telnet localhost 8888\\n");
    printf("3. Connect multiple clients and send messages\\n");
    printf("4. Type /quit to disconnect\\n\\n");
    
    /* Test Cases:
     * 1. Start server - Server should listen on specified port
     * 2. Connect client 1 - Should join and receive welcome message
     * 3. Connect client 2 - Both clients should see join notification
     * 4. Client 1 sends message - All clients should receive broadcast
     * 5. Client disconnects - All clients should see leave notification
     */
    
    start_server(PORT);
    
    return 0;
}
