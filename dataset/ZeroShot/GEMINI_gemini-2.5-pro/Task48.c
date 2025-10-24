#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 12345
#define MAX_CLIENTS 10
#define MAX_MSG_LENGTH 512

// Shared state must be protected by a mutex in a multi-threaded environment.
int client_sockets[MAX_CLIENTS] = {0};
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

// A simple function to add a client socket to our shared array.
void add_client(int sock) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] == 0) {
            client_sockets[i] = sock;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// A simple function to remove a client socket.
void remove_client(int sock) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] == sock) {
            client_sockets[i] = 0;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Broadcasts a message to all clients.
void broadcast_message(const char* message, int sender_sock) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] != 0) {
            // Error handling: Check if send fails.
            if (send(client_sockets[i], message, strlen(message), 0) < 0) {
                perror("send failed");
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Handles a single client connection. This function runs in its own thread.
void* handle_client(void* socket_desc) {
    int sock = *(int*)socket_desc;
    free(socket_desc); // Free the memory allocated in the server loop.

    char buffer[MAX_MSG_LENGTH + 1];
    char name[32];
    char message_to_broadcast[MAX_MSG_LENGTH + 50];

    // First message is the name.
    int read_size = recv(sock, name, sizeof(name) - 1, 0);
    if (read_size > 0) {
        name[read_size] = '\0';
        // Basic input sanitization: remove newline characters.
        name[strcspn(name, "\r\n")] = 0;
    } else {
        strcpy(name, "Anonymous");
    }

    printf("%s has connected.\n", name);
    snprintf(message_to_broadcast, sizeof(message_to_broadcast), "%s has joined.", name);
    broadcast_message(message_to_broadcast, sock);
    add_client(sock);

    // Loop to receive messages from the client.
    while ((read_size = recv(sock, buffer, MAX_MSG_LENGTH, 0)) > 0) {
        buffer[read_size] = '\0';
        // Security: Use snprintf to prevent buffer overflows when constructing the message.
        snprintf(message_to_broadcast, sizeof(message_to_broadcast), "%s: %s", name, buffer);
        broadcast_message(message_to_broadcast, sock);
        // Clear buffer after use to prevent data leakage from previous messages.
        memset(buffer, 0, sizeof(buffer));
    }
    
    // If recv returns 0 or -1, the client has disconnected.
    printf("%s has disconnected.\n", name);
    snprintf(message_to_broadcast, sizeof(message_to_broadcast), "%s has left.", name);
    broadcast_message(message_to_broadcast, sock);

    // Resource management: clean up.
    remove_client(sock);
    close(sock);
    return 0;
}

void* run_server(void* arg) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    while (1) {
        new_socket = accept(server_fd, NULL, NULL);
        if (new_socket < 0) {
            perror("accept");
            continue;
        }

        pthread_t client_thread;
        // Pass a copy of the socket descriptor to the thread.
        int* new_sock_ptr = malloc(sizeof(int));
        if (new_sock_ptr == NULL) {
            perror("malloc failed");
            close(new_socket);
            continue;
        }
        *new_sock_ptr = new_socket;

        if (pthread_create(&client_thread, NULL, handle_client, (void*)new_sock_ptr) < 0) {
            perror("could not create thread");
            free(new_sock_ptr);
        }
        // Detach thread to handle resource cleanup automatically on exit.
        pthread_detach(client_thread);
    }

    close(server_fd);
    return 0;
}

void* receive_messages(void* socket_desc) {
    int sock = *(int*)socket_desc;
    char server_reply[MAX_MSG_LENGTH + 50];
    while (recv(sock, server_reply, sizeof(server_reply) - 1, 0) > 0) {
        printf("%s\n", server_reply);
        memset(server_reply, 0, sizeof(server_reply));
    }
    return 0;
}

void run_client() {
    int sock;
    struct sockaddr_in serv_addr;
    char name[32];
    char message[MAX_MSG_LENGTH];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("Socket creation error \n");
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection Failed \n");
        return;
    }
    
    printf("Enter name: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = 0; // Remove newline
    send(sock, name, strlen(name), 0);
    
    pthread_t recv_thread;
    int* sock_ptr = malloc(sizeof(int));
    *sock_ptr = sock;
    pthread_create(&recv_thread, NULL, receive_messages, (void*)sock_ptr);

    while (fgets(message, sizeof(message), stdin) != NULL) {
        message[strcspn(message, "\n")] = 0; // Remove newline
        send(sock, message, strlen(message), 0);
    }

    close(sock);
    free(sock_ptr);
}

typedef struct {
    const char* name;
    const char* message;
} client_args;

void* run_test_client(void* args) {
    client_args* c_args = (client_args*)args;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) return NULL;
    
    send(sock, c_args->name, strlen(c_args->name), 0);
    usleep(200000); // 200ms
    send(sock, c_args->message, strlen(c_args->message), 0);
    usleep(500000); // 500ms
    close(sock);
    return NULL;
}

void run_test_cases() {
    printf("--- Running Automated Test Cases ---\n");
    // TC 1: Start Server
    pthread_t server_thread;
    pthread_create(&server_thread, NULL, run_server, NULL);
    sleep(1); // Wait for server to start up

    // TC 2 & 3: Two clients connect
    pthread_t c1, c2, c3;
    client_args args1 = {"ClientA", "Hello from C!"};
    client_args args2 = {"ClientB", "Testing..."};
    pthread_create(&c1, NULL, run_test_client, &args1);
    pthread_create(&c2, NULL, run_test_client, &args2);
    sleep(1);

    // TC 4: A third client connects
    client_args args3 = {"ClientC", "Third client here."};
    pthread_create(&c3, NULL, run_test_client, &args3);

    // TC 5: Wait for clients to finish
    pthread_join(c1, NULL);
    pthread_join(c2, NULL);
    pthread_join(c3, NULL);

    printf("\n--- Test Cases Finished ---\n");
    // In a real app, you'd signal the server to shut down cleanly.
    // For this test, we'll just exit, which is okay since it's a demo.
    // To stop server_thread, we could use pthread_cancel, but it's not clean.
    pthread_cancel(server_thread);
}


int main(int argc, char* argv[]) {
    if (argc > 1) {
        if (strcmp(argv[1], "server") == 0) {
            run_server(NULL);
        } else if (strcmp(argv[1], "client") == 0) {
            run_client();
        } else {
            fprintf(stderr, "Usage: %s [server|client]\n", argv[0]);
        }
    } else {
        run_test_cases();
        printf("To run manually: %s [server|client]\n", argv[0]);
    }
    return 0;
}