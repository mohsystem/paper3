
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <winsock2.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <pthread.h>
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket close
#endif

#define PORT 8888
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 100

typedef struct {
    SOCKET socket;
    char username[50];
    int active;
} ClientInfo;

ClientInfo clients[MAX_CLIENTS];
int client_count = 0;

#ifdef _WIN32
    CRITICAL_SECTION clients_mutex;
#else
    pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

void lock_clients() {
#ifdef _WIN32
    EnterCriticalSection(&clients_mutex);
#else
    pthread_mutex_lock(&clients_mutex);
#endif
}

void unlock_clients() {
#ifdef _WIN32
    LeaveCriticalSection(&clients_mutex);
#else
    pthread_mutex_unlock(&clients_mutex);
#endif
}

void broadcast_message(const char* message, SOCKET sender_socket) {
    lock_clients();
    for (int i = 0; i < client_count; i++) {
        if (clients[i].active && clients[i].socket != sender_socket) {
            send(clients[i].socket, message, strlen(message), 0);
        }
    }
    unlock_clients();
}

void remove_client(SOCKET socket) {
    lock_clients();
    for (int i = 0; i < client_count; i++) {
        if (clients[i].socket == socket) {
            clients[i].active = 0;
            break;
        }
    }
    unlock_clients();
}

#ifdef _WIN32
DWORD WINAPI handle_client(LPVOID arg) {
#else
void* handle_client(void* arg) {
#endif
    SOCKET client_socket = *(SOCKET*)arg;
    free(arg);
    
    char buffer[BUFFER_SIZE];
    char username[50] = {0};
    
    send(client_socket, "Enter your username:\\n", 21, 0);
    int bytes_read = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    
    if (bytes_read > 0) {
        buffer[bytes_read] = '\\0';
        sscanf(buffer, "%49s", username);
        
        if (strlen(username) == 0) {
            strcpy(username, "Anonymous");
        }
        
        lock_clients();
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (!clients[i].active) {
                clients[i].socket = client_socket;
                strcpy(clients[i].username, username);
                clients[i].active = 1;
                if (i >= client_count) client_count = i + 1;
                break;
            }
        }
        unlock_clients();
        
        printf("%s joined the chat\\n", username);
        
        char join_msg[BUFFER_SIZE];
        snprintf(join_msg, BUFFER_SIZE, "%s joined the chat\\n", username);
        broadcast_message(join_msg, client_socket);
        
        while (1) {
            bytes_read = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
            
            if (bytes_read <= 0) {
                break;
            }
            
            buffer[bytes_read] = '\\0';
            
            char* newline = strchr(buffer, '\\n');
            if (newline) *newline = '\\0';
            newline = strchr(buffer, '\\r');
            if (newline) *newline = '\\0';
            
            if (strcmp(buffer, "/quit") == 0) {
                break;
            }
            
            printf("%s: %s\\n", username, buffer);
            
            char msg[BUFFER_SIZE];
            snprintf(msg, BUFFER_SIZE, "%s: %s\\n", username, buffer);
            broadcast_message(msg, client_socket);
        }
        
        printf("%s left the chat\\n", username);
        
        char leave_msg[BUFFER_SIZE];
        snprintf(leave_msg, BUFFER_SIZE, "%s left the chat\\n", username);
        broadcast_message(leave_msg, client_socket);
    }
    
    remove_client(client_socket);
    closesocket(client_socket);
    
    return 0;
}

void start_server(int port) {
#ifdef _WIN32
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        fprintf(stderr, "WSAStartup failed\\n");
        return;
    }
    InitializeCriticalSection(&clients_mutex);
#endif
    
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        fprintf(stderr, "Socket creation failed\\n");
        return;
    }
    
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        fprintf(stderr, "Bind failed\\n");
        closesocket(server_socket);
        return;
    }
    
    if (listen(server_socket, 5) == SOCKET_ERROR) {
        fprintf(stderr, "Listen failed\\n");
        closesocket(server_socket);
        return;
    }
    
    printf("Chat Server starting on port %d\\n", port);
    printf("Server is listening on port %d\\n", port);
    
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        SOCKET client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        
        if (client_socket == INVALID_SOCKET) {
            fprintf(stderr, "Accept failed\\n");
            continue;
        }
        
        printf("New client connected: %s\\n", inet_ntoa(client_addr.sin_addr));
        
        SOCKET* socket_ptr = malloc(sizeof(SOCKET));
        *socket_ptr = client_socket;
        
#ifdef _WIN32
        CreateThread(NULL, 0, handle_client, socket_ptr, 0, NULL);
#else
        pthread_t thread;
        pthread_create(&thread, NULL, handle_client, socket_ptr);
        pthread_detach(thread);
#endif
    }
    
    closesocket(server_socket);
#ifdef _WIN32
    DeleteCriticalSection(&clients_mutex);
    WSACleanup();
#endif
}

int main() {
    start_server(PORT);
    return 0;
}
