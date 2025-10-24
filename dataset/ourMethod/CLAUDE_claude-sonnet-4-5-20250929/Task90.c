
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
    #define CLOSE_SOCKET closesocket
    typedef SOCKET socket_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <pthread.h>
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define CLOSE_SOCKET close
    typedef int socket_t;
#endif

/* Security limits to prevent resource exhaustion attacks */
#define MAX_MESSAGE_SIZE 4096
#define MAX_CLIENTS 100
#define MIN_PORT 1024
#define MAX_PORT 65535

/* Thread-safe client list with synchronization */
typedef struct {
    socket_t sockets[MAX_CLIENTS];
    size_t count;
#ifdef _WIN32
    CRITICAL_SECTION mutex;
#else
    pthread_mutex_t mutex;
#endif
} ClientList;

static ClientList g_clients;
static volatile sig_atomic_t g_server_running = 1;

/* Initialize client list with secure defaults */
void init_client_list(ClientList* list) {
    if (list == NULL) return;
    
    /* Initialize all sockets to invalid state */
    for (size_t i = 0; i < MAX_CLIENTS; i++) {
        list->sockets[i] = INVALID_SOCKET;
    }
    list->count = 0;
    
#ifdef _WIN32
    InitializeCriticalSection(&list->mutex);
#else
    pthread_mutex_init(&list->mutex, NULL);
#endif
}

/* Thread-safe client addition with bounds checking */
int add_client(ClientList* list, socket_t sock) {
    if (list == NULL || sock == INVALID_SOCKET) {
        return 0;
    }
    
#ifdef _WIN32
    EnterCriticalSection(&list->mutex);
#else
    pthread_mutex_lock(&list->mutex);
#endif
    
    /* Enforce maximum client limit */
    if (list->count >= MAX_CLIENTS) {
#ifdef _WIN32
        LeaveCriticalSection(&list->mutex);
#else
        pthread_mutex_unlock(&list->mutex);
#endif
        return 0;
    }
    
    /* Find first available slot with bounds check */
    for (size_t i = 0; i < MAX_CLIENTS; i++) {
        if (list->sockets[i] == INVALID_SOCKET) {
            list->sockets[i] = sock;
            list->count++;
#ifdef _WIN32
            LeaveCriticalSection(&list->mutex);
#else
            pthread_mutex_unlock(&list->mutex);
#endif
            return 1;
        }
    }
    
#ifdef _WIN32
    LeaveCriticalSection(&list->mutex);
#else
    pthread_mutex_unlock(&list->mutex);
#endif
    return 0;
}

/* Thread-safe client removal */
void remove_client(ClientList* list, socket_t sock) {
    if (list == NULL || sock == INVALID_SOCKET) {
        return;
    }
    
#ifdef _WIN32
    EnterCriticalSection(&list->mutex);
#else
    pthread_mutex_lock(&list->mutex);
#endif
    
    /* Search and remove with bounds check */
    for (size_t i = 0; i < MAX_CLIENTS; i++) {
        if (list->sockets[i] == sock) {
            list->sockets[i] = INVALID_SOCKET;
            if (list->count > 0) {
                list->count--;
            }
            break;
        }
    }
    
#ifdef _WIN32
    LeaveCriticalSection(&list->mutex);
#else
    pthread_mutex_unlock(&list->mutex);
#endif
}

/* Secure message broadcast with input validation */
void broadcast_message(ClientList* list, const char* msg, size_t msg_len, socket_t sender) {
    /* Validate inputs to prevent invalid memory access */
    if (list == NULL || msg == NULL || msg_len == 0 || msg_len > MAX_MESSAGE_SIZE) {
        return;
    }
    
    /* Allocate validated buffer on stack */
    char buffer[MAX_MESSAGE_SIZE + 1];
    
    /* Safe copy with explicit bounds check */
    size_t copy_len = msg_len < MAX_MESSAGE_SIZE ? msg_len : MAX_MESSAGE_SIZE;
    memcpy(buffer, msg, copy_len);
    buffer[copy_len] = '\\0';
    
#ifdef _WIN32
    EnterCriticalSection(&list->mutex);
#else
    pthread_mutex_lock(&list->mutex);
#endif
    
    /* Broadcast to all clients except sender with bounds checking */
    for (size_t i = 0; i < MAX_CLIENTS; i++) {
        if (list->sockets[i] != INVALID_SOCKET && list->sockets[i] != sender) {
            /* Send with error checking - ignore send failures for individual clients */
            send(list->sockets[i], buffer, (int)copy_len, 0);
        }
    }
    
#ifdef _WIN32
    LeaveCriticalSection(&list->mutex);
#else
    pthread_mutex_unlock(&list->mutex);
#endif
}

/* Thread argument structure for client handler */
typedef struct {
    socket_t socket;
    ClientList* clients;
} ThreadArg;

/* Secure client handler with proper resource management */
#ifdef _WIN32
DWORD WINAPI handle_client(LPVOID arg) {
#else
void* handle_client(void* arg) {
#endif
    if (arg == NULL) {
#ifdef _WIN32
        return 1;
#else
        return NULL;
#endif
    }
    
    ThreadArg* thread_arg = (ThreadArg*)arg;
    socket_t client_socket = thread_arg->socket;
    ClientList* clients = thread_arg->clients;
    
    /* Free argument immediately after copying to prevent memory leak */
    free(thread_arg);
    thread_arg = NULL;
    
    /* Fixed-size buffer to prevent unbounded memory growth */
    char buffer[MAX_MESSAGE_SIZE + 1];
    
    while (g_server_running) {
        /* Clear buffer before receive to prevent data leakage */
        memset(buffer, 0, sizeof(buffer));
        
        /* Receive with explicit size limit - never exceed buffer bounds */
        ssize_t bytes_recv = recv(client_socket, buffer, MAX_MESSAGE_SIZE, 0);
        
        /* Validate receive operation */
        if (bytes_recv <= 0) {
            break; /* Client disconnected or error */
        }
        
        /* Ensure null termination within bounds */
        if ((size_t)bytes_recv < sizeof(buffer)) {
            buffer[bytes_recv] = '\\0';
        } else {
            buffer[MAX_MESSAGE_SIZE] = '\\0';
        }
        
        /* Validate input contains only safe characters */
        int valid = 1;
        for (ssize_t i = 0; i < bytes_recv; i++) {
            if (buffer[i] != '\\n' && buffer[i] != '\\r' && 
                (buffer[i] < 32 || buffer[i] > 126)) {
                valid = 0;
                break;
            }
        }
        
        if (!valid) {
            continue; /* Skip invalid messages */
        }
        
        /* Broadcast validated message */
        broadcast_message(clients, buffer, (size_t)bytes_recv, client_socket);
    }
    
    /* Clean shutdown - remove client and close socket */
    remove_client(clients, client_socket);
    CLOSE_SOCKET(client_socket);
    
#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}

/* Secure server execution with comprehensive validation */
int run_server(int port) {
    /* Validate port range to prevent misconfigurations */
    if (port < MIN_PORT || port > MAX_PORT) {
        fprintf(stderr, "Port must be between %d and %d\\n", MIN_PORT, MAX_PORT);
        return 1;
    }
    
#ifdef _WIN32
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        fprintf(stderr, "WSAStartup failed\\n");
        return 1;
    }
#endif
    
    /* Initialize client list */
    init_client_list(&g_clients);
    
    /* Create socket with error checking */
    socket_t server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == INVALID_SOCKET) {
        fprintf(stderr, "Socket creation failed\\n");
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }
    
    /* Enable address reuse */
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, 
                   (const char*)&opt, sizeof(opt)) == SOCKET_ERROR) {
        fprintf(stderr, "Setsockopt failed\\n");
        CLOSE_SOCKET(server_socket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }
    
    /* Initialize address structure with secure defaults */
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons((uint16_t)port);
    
    /* Bind with error checking */
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        fprintf(stderr, "Bind failed\\n");
        CLOSE_SOCKET(server_socket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }
    
    /* Listen with reasonable backlog */
    if (listen(server_socket, 10) == SOCKET_ERROR) {
        fprintf(stderr, "Listen failed\\n");
        CLOSE_SOCKET(server_socket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }
    
    printf("Server listening on port %d\\n", port);
    
    while (g_server_running) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        memset(&client_addr, 0, sizeof(client_addr));
        
        /* Accept with validation */
        socket_t client_socket = accept(server_socket, 
                                       (struct sockaddr*)&client_addr, 
                                       &client_len);
        
        if (client_socket == INVALID_SOCKET) {
            continue;
        }
        
        /* Try to add client with limit enforcement */
        if (!add_client(&g_clients, client_socket)) {
            fprintf(stderr, "Maximum clients reached\\n");
            CLOSE_SOCKET(client_socket);
            continue;
        }
        
        /* Allocate thread argument - checked for NULL */
        ThreadArg* arg = (ThreadArg*)malloc(sizeof(ThreadArg));
        if (arg == NULL) {
            fprintf(stderr, "Memory allocation failed\\n");
            remove_client(&g_clients, client_socket);
            CLOSE_SOCKET(client_socket);
            continue;
        }
        
        arg->socket = client_socket;
        arg->clients = &g_clients;
        
        /* Create thread with error checking */
#ifdef _WIN32
        HANDLE thread = CreateThread(NULL, 0, handle_client, arg, 0, NULL);
        if (thread == NULL) {
            fprintf(stderr, "Thread creation failed\\n");
            free(arg);
            remove_client(&g_clients, client_socket);
            CLOSE_SOCKET(client_socket);
        } else {
            CloseHandle(thread);
        }
#else
        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_client, arg) != 0) {
            fprintf(stderr, "Thread creation failed\\n");
            free(arg);
            remove_client(&g_clients, client_socket);
            CLOSE_SOCKET(client_socket);
        } else {
            pthread_detach(thread);
        }
#endif
    }
    
    /* Clean shutdown */
    CLOSE_SOCKET(server_socket);
    
#ifdef _WIN32
    DeleteCriticalSection(&g_clients.mutex);
    WSACleanup();
#else
    pthread_mutex_destroy(&g_clients.mutex);
#endif
    
    return 0;
}

int main(void) {
    /* Test cases with validated ports */
    int test_ports[] = {8080, 8081, 8082, 8083, 8084};
    
    printf("Testing chat server with port: %d\\n", test_ports[0]);
    
    /* Run server on first test port */
    return run_server(test_ports[0]);
}
