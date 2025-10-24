#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#define close closesocket
typedef int socklen_t;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#endif

#define PORT 12348
#define BUFFER_SIZE 4096
#define MAX_KEY_LEN 256
#define MAX_VAL_LEN 1024

// Data structure for the remote object (a simple linked list)
typedef struct Node {
    char* key;
    char* value;
    struct Node* next;
} Node;

Node* head = NULL;
pthread_mutex_t remote_object_mutex = PTHREAD_MUTEX_INITIALIZER;
volatile int server_running = 1;

// A simple, thread-safe remote object server.
// NOTE: This implementation does not use TLS/SSL for encrypted communication
// or any form of authentication. In a production environment, these are critical.

void to_upper(char* str) {
    if (!str) return;
    for (int i = 0; str[i]; i++) {
        str[i] = toupper((unsigned char)str[i]);
    }
}

// Rule #1: Use locks to prevent race conditions (TOCTOU)
void put_value(const char* key, const char* value) {
    pthread_mutex_lock(&remote_object_mutex);
    Node* current = head;
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            free(current->value);
            current->value = strdup(value);
            pthread_mutex_unlock(&remote_object_mutex);
            return;
        }
        current = current->next;
    }
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (!newNode) { pthread_mutex_unlock(&remote_object_mutex); return; }
    newNode->key = strdup(key);
    newNode->value = strdup(value);
    if (!newNode->key || !newNode->value) {
        free(newNode->key);
        free(newNode->value);
        free(newNode);
        pthread_mutex_unlock(&remote_object_mutex);
        return;
    }
    newNode->next = head;
    head = newNode;
    pthread_mutex_unlock(&remote_object_mutex);
}

char* get_value(const char* key) {
    pthread_mutex_lock(&remote_object_mutex);
    Node* current = head;
    char* value_copy = NULL;
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            value_copy = strdup(current->value);
            break;
        }
        current = current->next;
    }
    pthread_mutex_unlock(&remote_object_mutex);
    return value_copy;
}

int delete_value(const char* key) {
    pthread_mutex_lock(&remote_object_mutex);
    Node *current = head, *prev = NULL;
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            if (prev == NULL) head = current->next;
            else prev->next = current->next;
            free(current->key);
            free(current->value);
            free(current);
            pthread_mutex_unlock(&remote_object_mutex);
            return 1; // Found and deleted
        }
        prev = current;
        current = current->next;
    }
    pthread_mutex_unlock(&remote_object_mutex);
    return 0; // Not found
}

void* handle_client(void* client_socket_ptr) {
    int client_socket = *(int*)client_socket_ptr;
    free(client_socket_ptr);
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        // Rule #6: Use boundary checks to prevent overflows.
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0) break;
        buffer[strcspn(buffer, "\r\n")] = 0;
        
        char *saveptr;
        char* command = strtok_r(buffer, " ", &saveptr);
        if (command == NULL) continue;
        
        to_upper(command);
        int should_exit = 0;

        if (strcmp(command, "PUT") == 0) {
            char* key = strtok_r(NULL, " ", &saveptr);
            char* value = strtok_r(NULL, "", &saveptr);
            if (key && value && strlen(key) <= MAX_KEY_LEN && strlen(value) <= MAX_VAL_LEN) {
                put_value(key, value);
                snprintf(response, sizeof(response), "OK\n");
            } else {
                snprintf(response, sizeof(response), "ERROR: PUT requires key and value, with valid lengths.\n");
            }
        } else if (strcmp(command, "GET") == 0) {
            char* key = strtok_r(NULL, " ", &saveptr);
            if (key && strlen(key) <= MAX_KEY_LEN) {
                char* val = get_value(key);
                if (val) {
                    snprintf(response, sizeof(response), "VALUE %s\n", val);
                    free(val);
                } else {
                    snprintf(response, sizeof(response), "NOT_FOUND\n");
                }
            } else {
                snprintf(response, sizeof(response), "ERROR: GET requires a key with valid length.\n");
            }
        } else if (strcmp(command, "DELETE") == 0) {
            char* key = strtok_r(NULL, " ", &saveptr);
            if (key && strlen(key) <= MAX_KEY_LEN) {
                if (delete_value(key)) snprintf(response, sizeof(response), "OK\n");
                else snprintf(response, sizeof(response), "NOT_FOUND\n");
            } else {
                snprintf(response, sizeof(response), "ERROR: DELETE requires a key with valid length.\n");
            }
        } else if (strcmp(command, "EXIT") == 0) {
            snprintf(response, sizeof(response), "GOODBYE\n");
            should_exit = 1;
        } else {
            snprintf(response, sizeof(response), "ERROR: Unknown command.\n");
        }
        send(client_socket, response, strlen(response), 0);
        if (should_exit) break;
    }
    
    close(client_socket);
    return NULL;
}

void* server_main(void* arg) {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) { perror("WSAStartup failed"); return NULL; }
#endif
    int server_socket;
    struct sockaddr_in server_addr;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) { perror("Socket creation failed"); return NULL; }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed"); close(server_socket); return NULL;
    }

    if (listen(server_socket, 5) < 0) { perror("Listen failed"); close(server_socket); return NULL; }
    printf("Server listening on port %d\n", PORT);

    while (server_running) {
        int client_socket = accept(server_socket, NULL, NULL);
        if (client_socket < 0) { if (server_running) perror("Accept failed"); continue; }
        
        pthread_t tid;
        int* pclient = malloc(sizeof(int));
        if (!pclient) { close(client_socket); continue; }
        *pclient = client_socket;
        if (pthread_create(&tid, NULL, handle_client, pclient) != 0) {
            perror("pthread_create failed"); free(pclient); close(client_socket);
        }
        pthread_detach(tid);
    }
    close(server_socket);
#ifdef _WIN32
    WSACleanup();
#endif
    printf("Server stopped.\n");
    return NULL;
}

char* send_request(const char* host, int port, const char* request) {
    int sock;
    struct sockaddr_in serv_addr;
    char* response_buffer = malloc(BUFFER_SIZE);
    if (!response_buffer) return NULL;
    memset(response_buffer, 0, BUFFER_SIZE);

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) { free(response_buffer); return NULL; }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    
    if(inet_pton(AF_INET, host, &serv_addr.sin_addr) <= 0) { close(sock); free(response_buffer); return NULL; }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) { close(sock); free(response_buffer); return NULL; }

    char req_buffer[BUFFER_SIZE];
    snprintf(req_buffer, sizeof(req_buffer), "%s\n", request);
    send(sock, req_buffer, strlen(req_buffer), 0);
    recv(sock, response_buffer, BUFFER_SIZE - 1, 0);
    response_buffer[strcspn(response_buffer, "\r\n")] = 0;

    close(sock);
    return response_buffer;
}

int main() {
    pthread_t server_thread_id;
    pthread_create(&server_thread_id, NULL, server_main, NULL);
    
#ifdef _WIN32
    Sleep(500);
#else
    usleep(500 * 1000);
#endif

    printf("--- Running Test Cases ---\n");
    char *res;

    printf("Test Case 1: PUT and GET\n");
    res = send_request("127.0.0.1", PORT, "PUT key1 value1");
    printf("Client sent: PUT key1 value1 -> Server responded: %s\n", res ? res : "ERROR"); free(res);
    res = send_request("127.0.0.1", PORT, "GET key1");
    printf("Client sent: GET key1 -> Server responded: %s\n\n", res ? res : "ERROR"); free(res);
    
    printf("Test Case 2: GET non-existent key\n");
    res = send_request("127.0.0.1", PORT, "GET non_existent_key");
    printf("Client sent: GET non_existent_key -> Server responded: %s\n\n", res ? res : "ERROR"); free(res);

    printf("Test Case 3: DELETE a key\n");
    res = send_request("127.0.0.1", PORT, "DELETE key1");
    printf("Client sent: DELETE key1 -> Server responded: %s\n", res ? res : "ERROR"); free(res);
    res = send_request("127.0.0.1", PORT, "GET key1");
    printf("Client sent: GET key1 -> Server responded: %s\n\n", res ? res : "ERROR"); free(res);

    printf("Test Case 4: Invalid command\n");
    res = send_request("127.0.0.1", PORT, "INVALID_COMMAND");
    printf("Client sent: INVALID_COMMAND -> Server responded: %s\n\n", res ? res : "ERROR"); free(res);
    
    printf("Test Case 5: Malformed command\n");
    res = send_request("127.0.0.1", PORT, "PUT key2");
    printf("Client sent: PUT key2 -> Server responded: %s\n\n", res ? res : "ERROR"); free(res);

    server_running = 0;
    res = send_request("127.0.0.1", PORT, "EXIT"); free(res);

    pthread_join(server_thread_id, NULL);

    pthread_mutex_lock(&remote_object_mutex);
    Node* current = head;
    while (current != NULL) {
        Node* temp = current;
        current = current->next;
        free(temp->key); free(temp->value); free(temp);
    }
    head = NULL;
    pthread_mutex_unlock(&remote_object_mutex);
    
    printf("--- Test Cases Finished ---\n");
    return 0;
}