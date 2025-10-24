
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
    #include <unistd.h>
    #include <arpa/inet.h>
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket close
#endif

#define MAX_OBJECTS 100
#define MAX_KEY_LEN 256
#define MAX_VALUE_LEN 1024

typedef struct {
    char key[MAX_KEY_LEN];
    char value[MAX_VALUE_LEN];
    int active;
} ObjectEntry;

ObjectEntry objectStore[MAX_OBJECTS];
pthread_mutex_t storeMutex = PTHREAD_MUTEX_INITIALIZER;

void storeObject(const char* key, const char* value) {
    pthread_mutex_lock(&storeMutex);
    
    int index = -1;
    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (objectStore[i].active && strcmp(objectStore[i].key, key) == 0) {
            index = i;
            break;
        }
    }
    
    if (index == -1) {
        for (int i = 0; i < MAX_OBJECTS; i++) {
            if (!objectStore[i].active) {
                index = i;
                break;
            }
        }
    }
    
    if (index != -1) {
        strncpy(objectStore[index].key, key, MAX_KEY_LEN - 1);
        strncpy(objectStore[index].value, value, MAX_VALUE_LEN - 1);
        objectStore[index].active = 1;
    }
    
    pthread_mutex_unlock(&storeMutex);
}

int retrieveObject(const char* key, char* value) {
    pthread_mutex_lock(&storeMutex);
    
    int found = 0;
    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (objectStore[i].active && strcmp(objectStore[i].key, key) == 0) {
            strcpy(value, objectStore[i].value);
            found = 1;
            break;
        }
    }
    
    pthread_mutex_unlock(&storeMutex);
    return found;
}

int deleteObject(const char* key) {
    pthread_mutex_lock(&storeMutex);
    
    int deleted = 0;
    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (objectStore[i].active && strcmp(objectStore[i].key, key) == 0) {
            objectStore[i].active = 0;
            deleted = 1;
            break;
        }
    }
    
    pthread_mutex_unlock(&storeMutex);
    return deleted;
}

void listKeys(char* keys) {
    pthread_mutex_lock(&storeMutex);
    
    strcpy(keys, "");
    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (objectStore[i].active) {
            strcat(keys, objectStore[i].key);
            strcat(keys, " ");
        }
    }
    
    pthread_mutex_unlock(&storeMutex);
}

void* handleClient(void* arg) {
    SOCKET clientSocket = *(SOCKET*)arg;
    free(arg);
    
    char buffer[4096];
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    
    if (bytesRead > 0) {
        buffer[bytesRead] = '\\0';
        
        char* command = strtok(buffer, "|");
        char response[4096];
        
        if (command != NULL) {
            if (strcmp(command, "STORE") == 0) {
                char* key = strtok(NULL, "|");
                char* value = strtok(NULL, "|");
                if (key != NULL && value != NULL) {
                    storeObject(key, value);
                    strcpy(response, "OK|Object stored");
                } else {
                    strcpy(response, "ERROR|Invalid parameters");
                }
            } else if (strcmp(command, "RETRIEVE") == 0) {
                char* key = strtok(NULL, "|");
                if (key != NULL) {
                    char value[MAX_VALUE_LEN];
                    if (retrieveObject(key, value)) {
                        sprintf(response, "OK|%s", value);
                    } else {
                        strcpy(response, "ERROR|Key not found");
                    }
                } else {
                    strcpy(response, "ERROR|Invalid parameters");
                }
            } else if (strcmp(command, "DELETE") == 0) {
                char* key = strtok(NULL, "|");
                if (key != NULL) {
                    if (deleteObject(key)) {
                        strcpy(response, "OK|Object deleted");
                    } else {
                        strcpy(response, "ERROR|Key not found");
                    }
                } else {
                    strcpy(response, "ERROR|Invalid parameters");
                }
            } else if (strcmp(command, "LIST") == 0) {
                char keys[4096];
                listKeys(keys);
                sprintf(response, "OK|%s", keys);
            } else {
                strcpy(response, "ERROR|Unknown command");
            }
            
            send(clientSocket, response, strlen(response), 0);
        }
    }
    
    closesocket(clientSocket);
    return NULL;
}

void* startServer(void* arg) {
    int port = *(int*)arg;
    
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
    
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        fprintf(stderr, "Failed to create socket\\n");
        return NULL;
    }
    
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);
    
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        fprintf(stderr, "Bind failed\\n");
        closesocket(serverSocket);
        return NULL;
    }
    
    if (listen(serverSocket, 5) == SOCKET_ERROR) {
        fprintf(stderr, "Listen failed\\n");
        closesocket(serverSocket);
        return NULL;
    }
    
    printf("Server listening on port %d\\n", port);
    
    while (1) {
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        
        if (clientSocket != INVALID_SOCKET) {
            SOCKET* socketPtr = malloc(sizeof(SOCKET));
            *socketPtr = clientSocket;
            
            pthread_t thread;
            pthread_create(&thread, NULL, handleClient, socketPtr);
            pthread_detach(thread);
        }
    }
    
    closesocket(serverSocket);
#ifdef _WIN32
    WSACleanup();
#endif
    
    return NULL;
}

int main() {
    memset(objectStore, 0, sizeof(objectStore));
    
    // Test cases
    printf("=== Test Case 1: Store and Retrieve ===\\n");
    storeObject("key1", "Hello World");
    char value[MAX_VALUE_LEN];
    if (retrieveObject("key1", value)) {
        printf("Retrieved: %s\\n", value);
    }
    
    printf("\\n=== Test Case 2: Store Multiple Objects ===\\n");
    storeObject("key2", "12345");
    storeObject("key3", "[1.1, 2.2, 3.3]");
    if (retrieveObject("key2", value)) {
        printf("Retrieved key2: %s\\n", value);
    }
    if (retrieveObject("key3", value)) {
        printf("Retrieved key3: %s\\n", value);
    }
    
    printf("\\n=== Test Case 3: Update Object ===\\n");
    storeObject("key1", "Updated Value");
    if (retrieveObject("key1", value)) {
        printf("Updated key1: %s\\n", value);
    }
    
    printf("\\n=== Test Case 4: Delete Object ===\\n");
    int deleted = deleteObject("key2");
    printf("Deleted key2: %s\\n", deleted ? "true" : "false");
    printf("Retrieve deleted key2: %s\\n", retrieveObject("key2", value) ? value : "not found");
    
    printf("\\n=== Test Case 5: List All Keys ===\\n");
    char keys[4096];
    listKeys(keys);
    printf("All keys: %s\\n", keys);
    
    // Start server in a separate thread
    int port = 8080;
    pthread_t serverThread;
    pthread_create(&serverThread, NULL, startServer, &port);
    pthread_detach(serverThread);
    
    printf("\\nServer started on port 8080. Press Enter to stop.\\n");
    getchar();
    
    return 0;
}
