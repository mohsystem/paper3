#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef SOCKET socket_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    typedef int socket_t;
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket close
#endif

#define PORT 12345
#define HOST "127.0.0.1"
#define BUFFER_SIZE 4096
#define MAX_CLIENTS 10

socket_t clients[MAX_CLIENTS];
char client_names[MAX_CLIENTS][32];
int client_count = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void broadcast_message(const char* message, socket_t sender_socket) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i] != sender_socket) {
            if (send(clients[i], message, strlen(message), 0) < 0) {
                perror("send failed");
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void remove_client(socket_t client_socket) {
    pthread_mutex_lock(&clients_mutex);
    int i;
    for (i = 0; i < client_count; i++) {
        if (clients[i] == client_socket) {
            break;
        }
    }
    // Shift remaining elements
    if (i < client_count) {
        for (int j = i; j < client_count - 1; j++) {
            clients[j] = clients[j + 1];
            strcpy(client_names[j], client_names[j+1]);
        }
        client_count--;
    }
    pthread_mutex_unlock(&clients_mutex);
}


void* handle_client(void* arg) {
    socket_t client_socket = *(socket_t*)arg;
    free(arg);
    char buffer[BUFFER_SIZE];
    char name[32];
    char message_buffer[BUFFER_SIZE + 40];
    int bytes_received;

    // Get client name
    const char* name_prompt = "Enter your name: ";
    send(client_socket, name_prompt, strlen(name_prompt), 0);
    bytes_received = recv(client_socket, name, 31, 0);
    if (bytes_received <= 0) {
        closesocket(client_socket);
        return NULL;
    }
    name[strcspn(name, "\r\n")] = 0; // Remove newline
    name[31] = '\0';

    printf("%s has connected.\n", name);
    
    // Add client to the list
    pthread_mutex_lock(&clients_mutex);
    if (client_count < MAX_CLIENTS) {
        clients[client_count] = client_socket;
        strncpy(client_names[client_count], name, 31);
        client_names[client_count][31] = '\0';
        int current_client_index = client_count;
        client_count++;
        pthread_mutex_unlock(&clients_mutex);
        
        snprintf(message_buffer, sizeof(message_buffer), "%s has joined the chat.", name);
        broadcast_message(message_buffer, client_socket);

        while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0)) > 0) {
            buffer[bytes_received] = '\0';
            buffer[strcspn(buffer, "\r\n")] = 0; // Remove newline

            if (strcmp(buffer, "exit") == 0) {
                break;
            }

            snprintf(message_buffer, sizeof(message_buffer), "%s: %s", name, buffer);
            printf("Broadcasting: %s\n", message_buffer);
            broadcast_message(message_buffer, client_socket);
        }

    } else {
        pthread_mutex_unlock(&clients_mutex);
        const char* full_msg = "Server is full. Try again later.\n";
        send(client_socket, full_msg, strlen(full_msg), 0);
    }
    
    // Cleanup
    remove_client(client_socket);
    printf("%s has disconnected.\n", name);
    snprintf(message_buffer, sizeof(message_buffer), "%s has left the chat.", name);
    broadcast_message(message_buffer, client_socket);
    closesocket(client_socket);
    return NULL;
}


void start_server() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed.\n");
        return;
    }
#endif

    socket_t server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        perror("Socket creation failed");
        return;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        perror("Bind failed");
        closesocket(server_socket);
        return;
    }

    if (listen(server_socket, 5) == SOCKET_ERROR) {
        perror("Listen failed");
        closesocket(server_socket);
        return;
    }

    printf("Server started on port %d. Waiting for connections...\n", PORT);

    while (true) {
        socket_t client_socket = accept(server_socket, NULL, NULL);
        if (client_socket == INVALID_SOCKET) {
            perror("Accept failed");
            continue;
        }
        
        pthread_t thread_id;
        socket_t* p_client_socket = malloc(sizeof(socket_t));
        if (p_client_socket == NULL) {
            perror("malloc failed");
            closesocket(client_socket);
            continue;
        }
        *p_client_socket = client_socket;
        
        if (pthread_create(&thread_id, NULL, handle_client, p_client_socket) != 0) {
            perror("pthread_create failed");
            free(p_client_socket);
            closesocket(client_socket);
        }
        pthread_detach(thread_id);
    }

    closesocket(server_socket);
#ifdef _WIN32
    WSACleanup();
#endif
}

void* receive_messages(void* arg) {
    socket_t sock = *(socket_t*)arg;
    char buffer[BUFFER_SIZE];
    int bytes_received;

    while ((bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        printf("%s\n", buffer);
    }

    if (bytes_received <= 0) {
        printf("Disconnected from server.\n");
        closesocket(sock);
        exit(0);
    }
    return NULL;
}

void start_client() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed.\n");
        return;
    }
#endif

    socket_t client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) {
        perror("Socket creation failed");
        return;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, HOST, &server_addr.sin_addr);

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        perror("Connection failed. Is the server running?");
        closesocket(client_socket);
        return;
    }
    
    printf("Connected to the chat server. Type 'exit' to quit.\n");
    
    char buffer[BUFFER_SIZE];
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        printf("%s", buffer);
        fgets(buffer, 32, stdin);
        send(client_socket, buffer, strlen(buffer), 0);
    }

    pthread_t recv_thread;
    if (pthread_create(&recv_thread, NULL, receive_messages, &client_socket) != 0) {
        perror("pthread_create for receiving failed");
        closesocket(client_socket);
        return;
    }
    pthread_detach(recv_thread);

    while (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
        buffer[strcspn(buffer, "\r\n")] = 0; // Remove newline
        send(client_socket, buffer, strlen(buffer), 0);
        if (strcmp(buffer, "exit") == 0) {
            break;
        }
    }

    closesocket(client_socket);
#ifdef _WIN32
    WSACleanup();
#endif
}

void print_usage() {
    printf("Usage: ./program_name <mode>\n");
    printf("  mode: 'server' or 'client'\n");
    printf("\n--- How to Test (5 Test Cases) ---\n");
    printf("1. Compile the code: gcc your_file.c -o program_name -pthread (on Windows, add -lws2_32)\n");
    printf("2. Start the server: Open a terminal and run './program_name server'\n");
    printf("3. Start Client 1: Open a second terminal and run './program_name client'. Enter a name.\n");
    printf("4. Start Client 2: Open a third terminal and run './program_name client'. Enter another name.\n");
    printf("5. Send Messages & Disconnect: Type messages to chat. Type 'exit' to disconnect a client.\n");
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage();
        return 1;
    }

    if (strcmp(argv[1], "server") == 0) {
        start_server();
    } else if (strcmp(argv[1], "client") == 0) {
        start_client();
    } else {
        print_usage();
    }

    return 0;
}