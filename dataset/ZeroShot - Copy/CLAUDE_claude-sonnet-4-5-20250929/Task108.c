
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#ifdef _WIN32
    #include <winsock2.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket close
#endif

#define PORT 8080
#define SECRET_KEY "MySecretKey12345"
#define MAX_OBJECTS 100
#define MAX_CLIENTS 50
#define BUFFER_SIZE 1024

typedef struct {
    char key[64];
    char value[256];
    int active;
} ServerObject;

typedef struct {
    char clientId[64];
    int authenticated;
} ClientInfo;

ServerObject serverObjects[MAX_OBJECTS];
ClientInfo authenticatedClients[MAX_CLIENTS];
pthread_mutex_t objectsMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t clientsMutex = PTHREAD_MUTEX_INITIALIZER;
int serverRunning = 0;

void initSocket() {
    #ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    #endif
}

void cleanupSocket() {
    #ifdef _WIN32
    WSACleanup();
    #endif
}

void sanitizeInput(char* input, char* output, int maxLen) {
    int j = 0;
    for (int i = 0; input[i] != '\\0' && j < maxLen - 1; i++) {
        char c = input[i];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || 
            (c >= '0' && c <= '9') || c == '_' || c == '-') {
            output[j++] = c;
        }
    }
    output[j] = '\\0';
}

void splitString(char* str, char delimiter, char tokens[][256], int* count) {
    *count = 0;
    char* token = strtok(str, &delimiter);
    while (token != NULL && *count < 10) {
        strcpy(tokens[*count], token);
        (*count)++;
        token = strtok(NULL, &delimiter);
    }
}

int authenticate(const char* token) {
    return strcmp(token, SECRET_KEY) == 0;
}

int isClientAuthenticated(const char* clientId) {
    pthread_mutex_lock(&clientsMutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (authenticatedClients[i].authenticated && 
            strcmp(authenticatedClients[i].clientId, clientId) == 0) {
            pthread_mutex_unlock(&clientsMutex);
            return 1;
        }
    }
    pthread_mutex_unlock(&clientsMutex);
    return 0;
}

void addAuthenticatedClient(const char* clientId) {
    pthread_mutex_lock(&clientsMutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!authenticatedClients[i].authenticated) {
            strcpy(authenticatedClients[i].clientId, clientId);
            authenticatedClients[i].authenticated = 1;
            break;
        }
    }
    pthread_mutex_unlock(&clientsMutex);
}

void processCommand(char tokens[][256], int count, char* response) {
    if (count == 0) {
        strcpy(response, "ERROR|Empty command");
        return;
    }
    
    if (strcmp(tokens[0], "GET") == 0 && count == 2) {
        char key[64];
        sanitizeInput(tokens[1], key, sizeof(key));
        
        pthread_mutex_lock(&objectsMutex);
        for (int i = 0; i < MAX_OBJECTS; i++) {
            if (serverObjects[i].active && strcmp(serverObjects[i].key, key) == 0) {
                sprintf(response, "SUCCESS|%s", serverObjects[i].value);
                pthread_mutex_unlock(&objectsMutex);
                return;
            }
        }
        pthread_mutex_unlock(&objectsMutex);
        strcpy(response, "ERROR|Object not found");
    }
    else if (strcmp(tokens[0], "SET") == 0 && count == 3) {
        char key[64], value[256];
        sanitizeInput(tokens[1], key, sizeof(key));
        sanitizeInput(tokens[2], value, sizeof(value));
        
        pthread_mutex_lock(&objectsMutex);
        int found = 0;
        for (int i = 0; i < MAX_OBJECTS; i++) {
            if (serverObjects[i].active && strcmp(serverObjects[i].key, key) == 0) {
                strcpy(serverObjects[i].value, value);
                found = 1;
                break;
            }
        }
        if (!found) {
            for (int i = 0; i < MAX_OBJECTS; i++) {
                if (!serverObjects[i].active) {
                    strcpy(serverObjects[i].key, key);
                    strcpy(serverObjects[i].value, value);
                    serverObjects[i].active = 1;
                    break;
                }
            }
        }
        pthread_mutex_unlock(&objectsMutex);
        strcpy(response, "SUCCESS|Object set");
    }
    else if (strcmp(tokens[0], "DELETE") == 0 && count == 2) {
        char key[64];
        sanitizeInput(tokens[1], key, sizeof(key));
        
        pthread_mutex_lock(&objectsMutex);
        for (int i = 0; i < MAX_OBJECTS; i++) {
            if (serverObjects[i].active && strcmp(serverObjects[i].key, key) == 0) {
                serverObjects[i].active = 0;
                break;
            }
        }
        pthread_mutex_unlock(&objectsMutex);
        strcpy(response, "SUCCESS|Object deleted");
    }
    else if (strcmp(tokens[0], "LIST") == 0) {
        char keys[1024] = "";
        pthread_mutex_lock(&objectsMutex);
        for (int i = 0; i < MAX_OBJECTS; i++) {
            if (serverObjects[i].active) {
                if (strlen(keys) > 0) strcat(keys, ",");
                strcat(keys, serverObjects[i].key);
            }
        }
        pthread_mutex_unlock(&objectsMutex);
        sprintf(response, "SUCCESS|%s", keys);
    }
    else {
        strcpy(response, "ERROR|Unknown command");
    }
}

typedef struct {
    SOCKET socket;
    char clientId[64];
} ClientThreadArgs;

void* handleClient(void* arg) {
    ClientThreadArgs* args = (ClientThreadArgs*)arg;
    SOCKET clientSocket = args->socket;
    char* clientId = args->clientId;
    
    char buffer[BUFFER_SIZE] = {0};
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    
    if (bytesRead > 0) {
        buffer[bytesRead] = '\\0';
        char tokens[10][256];
        int count;
        char bufferCopy[BUFFER_SIZE];
        strcpy(bufferCopy, buffer);
        splitString(bufferCopy, '|', tokens, &count);
        
        char response[BUFFER_SIZE];
        
        if (count > 0 && strcmp(tokens[0], "AUTH") == 0 && count == 2) {
            if (authenticate(tokens[1])) {
                addAuthenticatedClient(clientId);
                strcpy(response, "AUTH_SUCCESS");
            } else {
                strcpy(response, "AUTH_FAILED");
            }
        } else {
            if (!isClientAuthenticated(clientId)) {
                strcpy(response, "ERROR|Not authenticated");
            } else {
                processCommand(tokens, count, response);
            }
        }
        
        send(clientSocket, response, strlen(response), 0);
    }
    
    closesocket(clientSocket);
    free(args);
    return NULL;
}

void* serverThread(void* arg) {
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        printf("Socket creation failed\\n");
        return NULL;
    }
    
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);
    
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Bind failed\\n");
        closesocket(serverSocket);
        return NULL;
    }
    
    if (listen(serverSocket, 5) == SOCKET_ERROR) {
        printf("Listen failed\\n");
        closesocket(serverSocket);
        return NULL;
    }
    
    printf("Server started on port %d\\n", PORT);
    
    while (serverRunning) {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
        
        if (clientSocket != INVALID_SOCKET) {
            ClientThreadArgs* args = malloc(sizeof(ClientThreadArgs));
            args->socket = clientSocket;
            strcpy(args->clientId, inet_ntoa(clientAddr.sin_addr));
            
            pthread_t thread;
            pthread_create(&thread, NULL, handleClient, args);
            pthread_detach(thread);
        }
    }
    
    closesocket(serverSocket);
    cleanupSocket();
    return NULL;
}

void startServer() {
    initSocket();
    
    memset(serverObjects, 0, sizeof(serverObjects));
    memset(authenticatedClients, 0, sizeof(authenticatedClients));
    
    strcpy(serverObjects[0].key, "object1");
    strcpy(serverObjects[0].value, "value1");
    serverObjects[0].active = 1;
    
    strcpy(serverObjects[1].key, "object2");
    strcpy(serverObjects[1].value, "value2");
    serverObjects[1].active = 1;
    
    serverRunning = 1;
    
    pthread_t thread;
    pthread_create(&thread, NULL, serverThread, NULL);
    pthread_detach(thread);
}

void sendRequest(const char* host, int port, const char* request, char* response) {
    initSocket();
    
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        strcpy(response, "ERROR|Socket creation failed");
        return;
    }
    
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(host);
    
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        strcpy(response, "ERROR|Connection failed");
        closesocket(clientSocket);
        return;
    }
    
    send(clientSocket, request, strlen(request), 0);
    
    char buffer[BUFFER_SIZE] = {0};
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    
    closesocket(clientSocket);
    
    if (bytesRead > 0) {
        buffer[bytesRead] = '\\0';
        strcpy(response, buffer);
    } else {
        strcpy(response, "ERROR|No response");
    }
}

int main() {
    printf("=== Secure Remote Object Access System ===\\n\\n");
    
    startServer();
    
    #ifdef _WIN32
    Sleep(1000);
    #else
    sleep(1);
    #endif
    
    char response[BUFFER_SIZE];
    
    printf("Test 1 - Authentication:\\n");
    sendRequest("127.0.0.1", PORT, "AUTH|MySecretKey12345", response);
    printf("Response: %s\\n\\n", response);
    
    printf("Test 2 - Get object:\\n");
    sendRequest("127.0.0.1", PORT, "GET|object1", response);
    printf("Response: %s\\n\\n", response);
    
    printf("Test 3 - Set object:\\n");
    sendRequest("127.0.0.1", PORT, "SET|object3|value3", response);
    printf("Response: %s\\n\\n", response);
    
    printf("Test 4 - List objects:\\n");
    sendRequest("127.0.0.1", PORT, "LIST", response);
    printf("Response: %s\\n\\n", response);
    
    printf("Test 5 - Delete object:\\n");
    sendRequest("127.0.0.1", PORT, "DELETE|object2", response);
    printf("Response: %s\\n\\n", response);
    
    serverRunning = 0;
    
    #ifdef _WIN32
    Sleep(1000);
    #else
    sleep(1);
    #endif
    
    return 0;
}
