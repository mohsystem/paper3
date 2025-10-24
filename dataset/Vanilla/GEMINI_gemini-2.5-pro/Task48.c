#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef SOCKET socket_t;
    #define CLOSE_SOCKET(s) closesocket(s)
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    typedef int socket_t;
    #define CLOSE_SOCKET(s) close(s)
    #define INVALID_SOCKET -1
#endif

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define PORT 12345
#define HOST "127.0.0.1"

socket_t clients[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
volatile int server_running = 1;

void broadcast_message(const char* message, socket_t sender_socket) {
    pthread_mutex_lock(&clients_mutex);
    printf("Broadcasting: %s", message);
    for (int i = 0; i < client_count; i++) {
        if (clients[i] != sender_socket) {
            send(clients[i], message, strlen(message), 0);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void* handle_client(void* arg) {
    socket_t client_socket = *(socket_t*)arg;
    free(arg);
    char buffer[BUFFER_SIZE];
    char message_buffer[BUFFER_SIZE + 50];
    int bytes_received;

    sprintf(message_buffer, "Client-%ld has joined the chat.\n", (long)pthread_self());
    printf("%s", message_buffer);
    broadcast_message(message_buffer, client_socket);

    while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        sprintf(message_buffer, "Client-%ld: %s\n", (long)pthread_self(), buffer);
        broadcast_message(message_buffer, client_socket);
    }

    sprintf(message_buffer, "Client-%ld has left the chat.\n", (long)pthread_self());
    printf("%s", message_buffer);
    
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i] == client_socket) {
            for (int j = i; j < client_count - 1; j++) {
                clients[j] = clients[j + 1];
            }
            client_count--;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    
    broadcast_message(message_buffer, client_socket);
    CLOSE_SOCKET(client_socket);
    return NULL;
}

void* server_thread(void* arg) {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        perror("WSAStartup failed");
        return NULL;
    }
#endif
    socket_t server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        perror("Socket creation failed");
        return NULL;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, HOST, &server_addr.sin_addr);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        CLOSE_SOCKET(server_socket);
        return NULL;
    }

    if (listen(server_socket, 5) < 0) {
        perror("Listen failed");
        CLOSE_SOCKET(server_socket);
        return NULL;
    }
    printf("Server started on port %d\n", PORT);

    while (server_running) {
        socket_t client_socket = accept(server_socket, NULL, NULL);
        if (client_socket == INVALID_SOCKET) {
            if (server_running) perror("Accept failed");
            continue;
        }
        
        pthread_mutex_lock(&clients_mutex);
        if (client_count < MAX_CLIENTS) {
            clients[client_count++] = client_socket;
            pthread_t tid;
            socket_t* pclient = malloc(sizeof(socket_t));
            *pclient = client_socket;
            pthread_create(&tid, NULL, handle_client, pclient);
            pthread_detach(tid);
        } else {
            printf("Max clients reached. Connection rejected.\n");
            CLOSE_SOCKET(client_socket);
        }
        pthread_mutex_unlock(&clients_mutex);
    }
    
    CLOSE_SOCKET(server_socket);
#ifdef _WIN32
    WSACleanup();
#endif
    printf("Server stopped.\n");
    return NULL;
}

typedef struct {
    int id;
    const char** messages;
    int msg_count;
} client_args;

void* client_thread(void* arg) {
    client_args* args = (client_args*)arg;
    int id = args->id;

#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) { return NULL; }
#endif

    socket_t sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) { return NULL; }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, HOST, &serv_addr.sin_addr);
    
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Client-%d connection failed\n", id);
        CLOSE_SOCKET(sock);
        return NULL;
    }

    for (int i = 0; i < args->msg_count; i++) {
        send(sock, args->messages[i], strlen(args->messages[i]), 0);
        #ifdef _WIN32
        Sleep(500 + id * 100);
        #else
        usleep((500 + id * 100) * 1000);
        #endif
    }
    #ifdef _WIN32
    Sleep(1000);
    #else
    sleep(1);
    #endif

    CLOSE_SOCKET(sock);
    printf("Client-%d finished.\n", id);
#ifdef _WIN32
    WSACleanup();
#endif
    return NULL;
}

int main() {
    pthread_t server_tid;
    pthread_create(&server_tid, NULL, server_thread, NULL);
    #ifdef _WIN32
    Sleep(1000);
    #else
    sleep(1);
    #endif

    // Test cases: 5 clients
    pthread_t client_tids[5];
    client_args args[5];
    
    const char* msgs1[] = {"Hello everyone!", "How are you?"};
    const char* msgs2[] = {"Hi!", "I am fine, thanks!"};
    const char* msgs3[] = {"Good day.", "Any news?"};
    const char* msgs4[] = {"Hey there!", "Just joined."};
    const char* msgs5[] = {"Greetings.", "Let's chat."};
    
    args[0] = (client_args){1, msgs1, 2};
    args[1] = (client_args){2, msgs2, 2};
    args[2] = (client_args){3, msgs3, 2};
    args[3] = (client_args){4, msgs4, 2};
    args[4] = (client_args){5, msgs5, 2};

    // NOTE: This simulation does not print received messages for simplicity in C.
    // A real client would need a separate listener thread.
    printf("Starting 5 test clients...\n");
    for (int i = 0; i < 5; i++) {
        pthread_create(&client_tids[i], NULL, client_thread, &args[i]);
    }

    for (int i = 0; i < 5; i++) {
        pthread_join(client_tids[i], NULL);
    }

    printf("All clients finished. Stopping server...\n");
    server_running = 0;
    
    // Dummy connection to unblock accept()
    socket_t dummy_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, HOST, &server_addr.sin_addr);
    connect(dummy_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    CLOSE_SOCKET(dummy_socket);

    pthread_join(server_tid, NULL);
    printf("Chat simulation finished.\n");
    return 0;
}