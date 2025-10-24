
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>

/* Maximum message size to prevent excessive memory usage */
#define MAX_MESSAGE_SIZE 4096
/* Maximum number of clients to prevent resource exhaustion */
#define MAX_CLIENTS 100
/* Port number for the server */
#define SERVER_PORT 8888
/* Maximum username length */
#define MAX_USERNAME 32

/* Client structure to track connected clients */
typedef struct {
    int socket_fd;
    char username[MAX_USERNAME + 1]; /* +1 for null terminator */
    bool active;
    pthread_mutex_t lock;
} client_t;

/* Global client array with mutex for thread-safe access */
static client_t clients[MAX_CLIENTS];
static pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
static volatile sig_atomic_t server_running = 1;

/* Secure zeroization function that cannot be optimized away */
static void explicit_zero(void *ptr, size_t len) {
    if (ptr == NULL || len == 0) return;
    volatile unsigned char *p = (volatile unsigned char *)ptr;
    while (len--) *p++ = 0;
}

/* Initialize client structure with secure defaults */
static void init_client(client_t *client) {
    if (client == NULL) return;
    client->socket_fd = -1;
    explicit_zero(client->username, sizeof(client->username));
    client->active = false;
    pthread_mutex_init(&client->lock, NULL);
}

/* Safe string copy with null termination guarantee */
static int safe_strcpy(char *dest, size_t dest_size, const char *src) {
    /* Input validation: check for NULL pointers and valid size */
    if (dest == NULL || src == NULL || dest_size == 0) {
        return -1;
    }
    
    size_t i;
    /* Copy up to dest_size - 1 characters to ensure space for null terminator */
    for (i = 0; i < dest_size - 1 && src[i] != '\\0'; i++) {
        dest[i] = src[i];
    }
    /* Always null terminate */
    dest[i] = '\\0';
    
    return 0;
}

/* Validate input string: check length and reject control characters */
static bool validate_input(const char *input, size_t max_len) {
    if (input == NULL) return false;
    
    size_t len = 0;
    /* Check length and character validity */
    while (input[len] != '\\0') {
        /* Reject control characters except newline and tab */
        if ((input[len] < 32 && input[len] != '\\n' && input[len] != '\\t') || 
            input[len] == 127) {
            return false;
        }
        len++;
        /* Enforce maximum length to prevent buffer overflow */
        if (len >= max_len) {
            return false;
        }
    }
    
    return len > 0;
}

/* Add a new client to the clients array */
static int add_client(int socket_fd, const char *username) {
    /* Input validation */
    if (socket_fd < 0 || username == NULL) {
        return -1;
    }
    
    pthread_mutex_lock(&clients_mutex);
    
    int index = -1;
    /* Find an available slot */
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!clients[i].active) {
            index = i;
            break;
        }
    }
    
    if (index != -1) {
        pthread_mutex_lock(&clients[index].lock);
        clients[index].socket_fd = socket_fd;
        safe_strcpy(clients[index].username, sizeof(clients[index].username), username);
        clients[index].active = true;
        pthread_mutex_unlock(&clients[index].lock);
    }
    
    pthread_mutex_unlock(&clients_mutex);
    return index;
}

/* Remove a client from the clients array */
static void remove_client(int socket_fd) {
    pthread_mutex_lock(&clients_mutex);
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active && clients[i].socket_fd == socket_fd) {
            pthread_mutex_lock(&clients[i].lock);
            clients[i].active = false;
            clients[i].socket_fd = -1;
            explicit_zero(clients[i].username, sizeof(clients[i].username));
            pthread_mutex_unlock(&clients[i].lock);
            break;
        }
    }
    
    pthread_mutex_unlock(&clients_mutex);
}

/* Broadcast message to all connected clients except sender */
static void broadcast_message(const char *message, int sender_fd) {
    /* Input validation */
    if (message == NULL) return;
    
    size_t msg_len = strnlen(message, MAX_MESSAGE_SIZE);
    if (msg_len == 0 || msg_len >= MAX_MESSAGE_SIZE) return;
    
    pthread_mutex_lock(&clients_mutex);
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active && clients[i].socket_fd != sender_fd) {
            pthread_mutex_lock(&clients[i].lock);
            /* Check all send return values */
            ssize_t sent = send(clients[i].socket_fd, message, msg_len, MSG_NOSIGNAL);
            if (sent < 0) {
                /* Log error but continue to other clients - fail open for broadcast */
                fprintf(stderr, "Failed to send to client: %s\\n", strerror(errno));
            }
            pthread_mutex_unlock(&clients[i].lock);
        }
    }
    
    pthread_mutex_unlock(&clients_mutex);
}

/* Thread function to handle individual client */
static void *handle_client(void *arg) {
    int client_fd = *(int *)arg;
    free(arg); /* Free the allocated socket descriptor */
    
    char username[MAX_USERNAME + 1];
    char buffer[MAX_MESSAGE_SIZE];
    char formatted_msg[MAX_MESSAGE_SIZE + MAX_USERNAME + 32];
    ssize_t bytes_received;
    
    /* Initialize buffers to zero */
    explicit_zero(username, sizeof(username));
    explicit_zero(buffer, sizeof(buffer));
    
    /* Receive username from client with bounds checking */
    bytes_received = recv(client_fd, username, MAX_USERNAME, 0);
    if (bytes_received <= 0 || bytes_received > MAX_USERNAME) {
        close(client_fd);
        return NULL;
    }
    
    /* Null terminate and validate username */
    username[bytes_received] = '\\0';
    if (!validate_input(username, MAX_USERNAME)) {
        const char *error_msg = "Invalid username\\n";
        send(client_fd, error_msg, strlen(error_msg), MSG_NOSIGNAL);
        close(client_fd);
        return NULL;
    }
    
    /* Add client to the list */
    int client_index = add_client(client_fd, username);
    if (client_index == -1) {
        const char *error_msg = "Server full\\n";
        send(client_fd, error_msg, strlen(error_msg), MSG_NOSIGNAL);
        close(client_fd);
        return NULL;
    }
    
    /* Announce new client */
    snprintf(formatted_msg, sizeof(formatted_msg), "[Server] %s joined the chat\\n", username);
    broadcast_message(formatted_msg, -1);
    
    /* Main message handling loop */
    while (server_running) {
        explicit_zero(buffer, sizeof(buffer));
        
        /* Receive message with size limit to prevent overflow */
        bytes_received = recv(client_fd, buffer, MAX_MESSAGE_SIZE - 1, 0);
        
        if (bytes_received <= 0) {
            /* Client disconnected or error occurred */
            break;
        }
        
        /* Ensure null termination */
        buffer[bytes_received] = '\\0';
        
        /* Validate received message */
        if (!validate_input(buffer, MAX_MESSAGE_SIZE)) {
            continue; /* Skip invalid messages */
        }
        
        /* Format and broadcast message */
        int result = snprintf(formatted_msg, sizeof(formatted_msg), "[%s]: %s", username, buffer);
        if (result > 0 && (size_t)result < sizeof(formatted_msg)) {
            broadcast_message(formatted_msg, client_fd);
        }
    }
    
    /* Clean up client */
    snprintf(formatted_msg, sizeof(formatted_msg), "[Server] %s left the chat\\n", username);
    broadcast_message(formatted_msg, -1);
    
    remove_client(client_fd);
    close(client_fd);
    
    /* Clear sensitive data before thread exit */
    explicit_zero(username, sizeof(username));
    explicit_zero(buffer, sizeof(buffer));
    
    return NULL;
}

/* Signal handler for graceful shutdown */
static void signal_handler(int sig) {
    (void)sig; /* Unused parameter */
    server_running = 0;
}

/* Server main function */
static int run_server(void) {
    int server_fd = -1;
    int client_fd = -1;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    /* Initialize all clients */
    for (int i = 0; i < MAX_CLIENTS; i++) {
        init_client(&clients[i]);
    }
    
    /* Set up signal handler for graceful shutdown */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    /* Create socket */
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        fprintf(stderr, "Failed to create socket\\n");
        return 1;
    }
    
    /* Set socket option to reuse address */
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        fprintf(stderr, "Failed to set socket options\\n");
        close(server_fd);
        return 1;
    }
    
    /* Initialize server address structure */
    explicit_zero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);
    
    /* Bind socket */
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "Failed to bind socket\\n");
        close(server_fd);
        return 1;
    }
    
    /* Listen for connections */
    if (listen(server_fd, 10) < 0) {
        fprintf(stderr, "Failed to listen\\n");
        close(server_fd);
        return 1;
    }
    
    printf("Server listening on port %d\\n", SERVER_PORT);
    
    /* Accept client connections */
    while (server_running) {
        explicit_zero(&client_addr, sizeof(client_addr));
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        
        if (client_fd < 0) {
            if (errno == EINTR) continue; /* Interrupted by signal */
            fprintf(stderr, "Failed to accept connection\\n");
            continue;
        }
        
        /* Allocate memory for client socket descriptor */
        int *client_sock = (int *)malloc(sizeof(int));
        if (client_sock == NULL) {
            fprintf(stderr, "Memory allocation failed\\n");
            close(client_fd);
            continue;
        }
        *client_sock = client_fd;
        
        /* Create thread to handle client */
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, (void *)client_sock) != 0) {
            fprintf(stderr, "Failed to create thread\\n");
            free(client_sock);
            close(client_fd);
            continue;
        }
        
        /* Detach thread so resources are cleaned up automatically */
        pthread_detach(thread_id);
    }
    
    /* Clean up */
    close(server_fd);
    
    /* Close all client connections */
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active) {
            close(clients[i].socket_fd);
            pthread_mutex_destroy(&clients[i].lock);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    pthread_mutex_destroy(&clients_mutex);
    
    return 0;
}

/* Client function for testing */
static int run_client(const char *username) {
    int sock_fd = -1;
    struct sockaddr_in server_addr;
    char buffer[MAX_MESSAGE_SIZE];
    fd_set read_fds;
    
    /* Validate username */
    if (username == NULL || !validate_input(username, MAX_USERNAME)) {
        fprintf(stderr, "Invalid username\\n");
        return 1;
    }
    
    /* Create socket */
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        fprintf(stderr, "Failed to create socket\\n");
        return 1;
    }
    
    /* Initialize server address */
    explicit_zero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    
    /* Convert IP address */
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid address\\n");
        close(sock_fd);
        return 1;
    }
    
    /* Connect to server */
    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "Connection failed\\n");
        close(sock_fd);
        return 1;
    }
    
    /* Send username */
    if (send(sock_fd, username, strnlen(username, MAX_USERNAME), 0) < 0) {
        fprintf(stderr, "Failed to send username\\n");
        close(sock_fd);
        return 1;
    }
    
    printf("Connected as %s. Type messages to send:\\n", username);
    
    /* Main client loop */
    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        FD_SET(sock_fd, &read_fds);
        
        int max_fd = sock_fd > STDIN_FILENO ? sock_fd : STDIN_FILENO;
        
        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0) {
            if (errno == EINTR) continue;
            break;
        }
        
        /* Check for server messages */
        if (FD_ISSET(sock_fd, &read_fds)) {
            explicit_zero(buffer, sizeof(buffer));
            ssize_t bytes = recv(sock_fd, buffer, MAX_MESSAGE_SIZE - 1, 0);
            if (bytes <= 0) {
                printf("Disconnected from server\\n");
                break;
            }
            buffer[bytes] = '\\0';
            printf("%s", buffer);
            fflush(stdout);
        }
        
        /* Check for user input */
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            explicit_zero(buffer, sizeof(buffer));
            if (fgets(buffer, MAX_MESSAGE_SIZE - 1, stdin) == NULL) {
                break;
            }
            
            /* Validate and send message */
            if (validate_input(buffer, MAX_MESSAGE_SIZE)) {
                if (send(sock_fd, buffer, strnlen(buffer, MAX_MESSAGE_SIZE), 0) < 0) {
                    fprintf(stderr, "Failed to send message\\n");
                    break;
                }
            }
        }
    }
    
    close(sock_fd);
    explicit_zero(buffer, sizeof(buffer));
    return 0;
}

int main(int argc, char *argv[]) {
    /* Validate command line arguments */
    if (argc < 2) {
        printf("Usage:\\n");
        printf("  Server mode: %s server\\n", argv[0]);
        printf("  Client mode: %s client <username>\\n", argv[0]);
        return 1;
    }
    
    if (strcmp(argv[1], "server") == 0) {
        return run_server();
    } else if (strcmp(argv[1], "client") == 0 && argc >= 3) {
        return run_client(argv[2]);
    } else {
        printf("Invalid arguments\\n");
        return 1;
    }
}
