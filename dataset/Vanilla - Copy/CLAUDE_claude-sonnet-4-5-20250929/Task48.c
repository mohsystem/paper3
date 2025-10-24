
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef SOCKET socket_t;
    #define CLOSE_SOCKET closesocket
    #define sleep(x) Sleep((x) * 1000)
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    typedef int socket_t;
    #define CLOSE_SOCKET close
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
#endif

#define PORT 12345
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

typedef struct {
    socket_t socket;
    char name[100];
    int active;
} ClientHandler;

ClientHandler clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
int client_count = 0;

void broadcast(const char* message) {
    printf("[Server] %s\\n", message);
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i].active) {
            char msg[BUFFER_SIZE];
            snprintf(msg, sizeof(msg), "%s\\n", message);
            send(clients[i].socket, msg, strlen(msg), 0);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void remove_client(int index) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = index; i < client_count - 1; i++) {
        clients[i] = clients[i + 1];
    }
    client_count--;
    pthread_mutex_unlock(&clients_mutex);
}

void* handle_client(void* arg) {
    ClientHandler* handler = (ClientHandler*)arg;
    char buffer[BUFFER_SIZE];
    
    send(handler->socket, "Enter your name:\\n", 17, 0);
    int bytes = recv(handler->socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes > 0) {
        buffer[bytes] = '\\0';
        sscanf(buffer, "%s", handler->name);
        char join_msg[BUFFER_SIZE];
        snprintf(join_msg, sizeof(join_msg), "%s has joined the chat!", handler->name);
        broadcast(join_msg);
    }
    
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        bytes = recv(handler->socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) break;
        
        buffer[bytes] = '\\0';
        char* newline = strchr(buffer, '\\n');
        if (newline) *newline = '\\0';
        newline = strchr(buffer, '\\r');
        if (newline) *newline = '\\0';
        
        if (strcmp(buffer, "/quit") == 0) break;
        
        char msg[BUFFER_SIZE];
        snprintf(msg, sizeof(msg), "%s: %s", handler->name, buffer);
        broadcast(msg);
    }
    
    handler->active = 0;
    char leave_msg[BUFFER_SIZE];
    snprintf(leave_msg, sizeof(leave_msg), "%s has left the chat!", handler->name);
    broadcast(leave_msg);
    
    CLOSE_SOCKET(handler->socket);
    return NULL;
}

void* start_server(void* arg) {
    #ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    #endif
    
    printf("Chat server starting on port %d\\n", PORT);
    
    socket_t server_socket = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_socket, 5);
    
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        socket_t client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        
        if (client_socket == INVALID_SOCKET) continue;
        
        pthread_mutex_lock(&clients_mutex);
        if (client_count < MAX_CLIENTS) {
            clients[client_count].socket = client_socket;
            clients[client_count].active = 1;
            
            pthread_t thread;
            pthread_create(&thread, NULL, handle_client, &clients[client_count]);
            pthread_detach(thread);
            
            client_count++;
        }
        pthread_mutex_unlock(&clients_mutex);
    }
    
    return NULL;
}

typedef struct {
    socket_t sock;
    int running;
} ChatClient;

void* receive_messages(void* arg) {
    ChatClient* client = (ChatClient*)arg;
    char buffer[BUFFER_SIZE];
    
    while (client->running) {
        memset(buffer, 0, sizeof(buffer));
        int bytes = recv(client->sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes > 0) {
            printf("%s", buffer);
        } else {
            break;
        }
    }
    return NULL;
}

ChatClient* create_client() {
    ChatClient* client = (ChatClient*)malloc(sizeof(ChatClient));
    client->sock = INVALID_SOCKET;
    client->running = 0;
    return client;
}

void connect_client(ChatClient* client, const char* host, int port, const char* name) {
    #ifdef _WIN32
    static int wsa_initialized = 0;
    if (!wsa_initialized) {
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
        wsa_initialized = 1;
    }
    #endif
    
    client->sock = socket(AF_INET, SOCK_STREAM, 0);
    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, host, &server_addr.sin_addr);
    
    connect(client->sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    client->running = 1;
    
    char buffer[BUFFER_SIZE];
    recv(client->sock, buffer, sizeof(buffer), 0);
    printf("%s", buffer);
    
    char name_msg[BUFFER_SIZE];
    snprintf(name_msg, sizeof(name_msg), "%s\\n", name);
    send(client->sock, name_msg, strlen(name_msg), 0);
    
    pthread_t thread;
    pthread_create(&thread, NULL, receive_messages, client);
    pthread_detach(thread);
}

void send_message_client(ChatClient* client, const char* message) {
    char msg[BUFFER_SIZE];
    snprintf(msg, sizeof(msg), "%s\\n", message);
    send(client->sock, msg, strlen(msg), 0);
}

void disconnect_client(ChatClient* client) {
    client->running = 0;
    if (client->sock != INVALID_SOCKET) {
        CLOSE_SOCKET(client->sock);
    }
}

int main() {
    pthread_t server_thread;
    pthread_create(&server_thread, NULL, start_server, NULL);
    pthread_detach(server_thread);
    
    sleep(1);
    
    printf("\\n=== Test Case 1: Single client connection ===\\n");
    ChatClient* client1 = create_client();
    connect_client(client1, "127.0.0.1", PORT, "Alice");
    usleep(500000);
    send_message_client(client1, "Hello from Alice!");
    usleep(500000);
    
    printf("\\n=== Test Case 2: Multiple clients ===\\n");
    ChatClient* client2 = create_client();
    connect_client(client2, "127.0.0.1", PORT, "Bob");
    usleep(500000);
    send_message_client(client2, "Hi everyone from Bob!");
    usleep(500000);
    
    printf("\\n=== Test Case 3: Broadcasting ===\\n");
    send_message_client(client1, "This is Alice speaking");
    usleep(500000);
    send_message_client(client2, "Bob here too");
    usleep(500000);
    
    printf("\\n=== Test Case 4: Third client joins ===\\n");
    ChatClient* client3 = create_client();
    connect_client(client3, "127.0.0.1", PORT, "Charlie");
    usleep(500000);
    send_message_client(client3, "Charlie joined the chat");
    usleep(500000);
    
    printf("\\n=== Test Case 5: Client disconnection ===\\n");
    send_message_client(client1, "/quit");
    usleep(500000);
    send_message_client(client2, "Alice left");
    usleep(500000);
    
    disconnect_client(client2);
    disconnect_client(client3);
    sleep(1);
    
    free(client1);
    free(client2);
    free(client3);
    
    return 0;
}
