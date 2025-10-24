#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/wait.h>

#define MAX_CLIENTS 100
#define BUFFER_SIZE 2048

// Client state structure
typedef struct {
    int sockfd;
    struct sockaddr_in address;
    int uid;
} client_t;

// Global variables for client management
static client_t *clients[MAX_CLIENTS];
static int uid_counter = 10;
static int client_count = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

// Helper to remove trailing newline characters from a string
void str_trim_crlf(char* str) {
    if (str == NULL) return;
    int len = strlen(str);
    while (len > 0 && (str[len-1] == '\n' || str[len-1] == '\r')) {
        str[--len] = '\0';
    }
}

// Function to safely add a client to the global array. Returns 1 on success, 0 on failure.
int add_client(client_t *cl) {
    pthread_mutex_lock(&clients_mutex);
    if (client_count >= MAX_CLIENTS) {
        pthread_mutex_unlock(&clients_mutex);
        return 0;
    }
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i] == NULL) {
            clients[i] = cl;
            client_count++;
            pthread_mutex_unlock(&clients_mutex);
            return 1;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    return 0; // Should not be reached if client_count is managed correctly
}

// Function to safely remove a client from the global array
void remove_client(int uid) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i] && clients[i]->uid == uid) {
            clients[i] = NULL;
            client_count--;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Function to send a message to all clients except the sender
void broadcast_message(const char *message, int sender_uid) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i] && clients[i]->uid != sender_uid) {
            if (write(clients[i]->sockfd, message, strlen(message)) < 0) {
                // This might fail if client disconnected. The client's own thread will handle the cleanup.
                perror("write to client failed");
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Function to handle communication with a single client in a separate thread
void *handle_client(void *arg) {
    char buffer[BUFFER_SIZE];
    char message_with_sender[BUFFER_SIZE + 64]; // Extra space for "[User UID]: \n\0"
    int leave_flag = 0;
    ssize_t read_len;

    client_t *cli = (client_t *)arg;

    // Announce new client to others
    snprintf(message_with_sender, sizeof(message_with_sender), "User %d has joined\n", cli->uid);
    printf("%s", message_with_sender);
    broadcast_message(message_with_sender, cli->uid);

    // Loop to receive and broadcast messages
    while ((read_len = recv(cli->sockfd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        if (leave_flag) {
            break;
        }
        
        buffer[read_len] = '\0';
        str_trim_crlf(buffer);

        if (strlen(buffer) == 0) {
            continue; // Skip empty messages
        }

        // Check for exit command
        if (strcmp(buffer, "exit") == 0) {
            snprintf(message_with_sender, sizeof(message_with_sender), "User %d has left\n", cli->uid);
            printf("%s", message_with_sender);
            broadcast_message(message_with_sender, cli->uid);
            leave_flag = 1;
        } else {
            snprintf(message_with_sender, sizeof(message_with_sender), "[User %d]: %s\n", cli->uid, buffer);
            printf("%s", message_with_sender);
            broadcast_message(message_with_sender, cli->uid);
        }
    }

    // Cleanup when client disconnects or an error occurs
    close(cli->sockfd);
    remove_client(cli->uid);
    free(cli);
    return NULL;
}

// Main function to run the server
int run_server(int port) {
    int listenfd = 0;
    struct sockaddr_in serv_addr;
    pthread_t tid;

    // Ignore SIGPIPE to prevent server crash when writing to a closed socket
    signal(SIGPIPE, SIG_IGN);

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        perror("socket failed");
        return EXIT_FAILURE;
    }

    int opt = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        close(listenfd);
        return EXIT_FAILURE;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    if (bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind failed");
        close(listenfd);
        return EXIT_FAILURE;
    }

    if (listen(listenfd, 10) < 0) {
        perror("listen failed");
        close(listenfd);
        return EXIT_FAILURE;
    }

    printf("=== CHAT SERVER STARTED ON PORT %d ===\n", port);

    while(1) {
        struct sockaddr_in cli_addr;
        socklen_t clilen = sizeof(cli_addr);
        int connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &clilen);

        if (connfd < 0) {
            perror("accept failed");
            continue;
        }
        
        client_t *cli = (client_t *)malloc(sizeof(client_t));
        if (cli == NULL) {
            perror("malloc for client_t failed");
            close(connfd);
            continue;
        }
        cli->address = cli_addr;
        cli->sockfd = connfd;
        cli->uid = uid_counter++;

        if (!add_client(cli)) {
            fprintf(stderr, "Max clients reached. Connection rejected.\n");
            free(cli);
            close(connfd);
            continue;
        }
        
        if (pthread_create(&tid, NULL, &handle_client, (void*)cli) != 0) {
            perror("pthread_create failed");
            remove_client(cli->uid);
            free(cli);
            close(connfd);
        }
        pthread_detach(tid);
    }

    close(listenfd);
    return EXIT_SUCCESS;
}

// Test function to simulate a client
void run_test_client(int port, const char* name, const char* messages[], int num_messages) {
    int sockfd = 0;
    struct sockaddr_in serv_addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "\n[%s] Error : Could not create socket \n", name);
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        fprintf(stderr, "[%s] Error: Invalid address\n", name);
        close(sockfd);
        return;
    }

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        fprintf(stderr, "\n[%s] Error : Connect Failed \n", name);
        close(sockfd);
        return;
    }

    printf("[%s] Connected to server.\n", name);
    sleep(1); 

    for (int i = 0; i < num_messages; i++) {
        if (send(sockfd, messages[i], strlen(messages[i]), 0) < 0) {
            perror("send failed");
            break;
        }
        printf("[%s] sent: %s\n", name, messages[i]);
        sleep(1);
    }
    
    const char* exit_msg = "exit";
    if (send(sockfd, exit_msg, strlen(exit_msg), 0) < 0) {
        perror("send exit failed");
    }
    printf("[%s] sent: %s\n", name, exit_msg);

    close(sockfd);
    printf("[%s] Connection closed.\n", name);
}

// Main entry point
int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <port> [--test]\n", argv[0]);
        return EXIT_FAILURE;
    }

    char* endptr;
    long port_long = strtol(argv[1], &endptr, 10);
    if (*endptr != '\0' || port_long <= 0 || port_long > 65535) {
        fprintf(stderr, "Invalid port number '%s'. Must be between 1 and 65535.\n", argv[1]);
        return EXIT_FAILURE;
    }
    int port = (int)port_long;

    // Optional test mode with 5 client test cases
    if (argc == 3 && strcmp(argv[2], "--test") == 0) {
        printf("Running in test mode.\n");

        pid_t server_pid = fork();
        if (server_pid < 0) {
            perror("fork for server failed");
            return EXIT_FAILURE;
        } else if (server_pid == 0) {
            // Child process: run the server
            if (run_server(port) != EXIT_SUCCESS) {
               exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
        } else {
            // Parent process: run test clients
            sleep(2); // Give server time to initialize

            const char* client1_msgs[] = {"Hello everyone!", "How is it going?"};
            const char* client2_msgs[] = {"Hi there!", "I'm doing great, thanks!"};
            const char* client3_msgs[] = {"Hey, I'm new here."};
            const char* client4_msgs[] = {"Welcome!", "This is a test chat."};
            const char* client5_msgs[] = {"Testing... 1, 2, 3."};

            const char** all_msgs[] = {client1_msgs, client2_msgs, client3_msgs, client4_msgs, client5_msgs};
            const int num_msgs[] = {2, 2, 1, 2, 1};
            char client_names[5][10];

            pid_t client_pids[5];
            for(int i=0; i<5; i++) client_pids[i] = 0;

            for (int i = 0; i < 5; i++) {
                snprintf(client_names[i], 10, "Client%d", i + 1);
                client_pids[i] = fork();
                if (client_pids[i] < 0) {
                    perror("fork for client failed");
                    break; 
                } else if (client_pids[i] == 0) {
                    run_test_client(port, client_names[i], all_msgs[i], num_msgs[i]);
                    exit(0);
                }
            }
            
            for (int i = 0; i < 5; i++) {
                if (client_pids[i] > 0) {
                    waitpid(client_pids[i], NULL, 0);
                }
            }

            printf("\nTest finished. Stopping server.\n");
            if (kill(server_pid, SIGTERM) == -1) {
                perror("kill server failed");
            }
            waitpid(server_pid, NULL, 0);
            printf("Server stopped.\n");
        }
    } else {
        run_server(port);
    }

    return EXIT_SUCCESS;
}