
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
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

#define PORT 8443
#define MAX_CONNECTIONS 10
#define SESSION_TIMEOUT 300
#define MAX_STORE_SIZE 100

/* Note: This is a simplified single-threaded demonstration in C.
   For production use, implement proper multi-threading, mutex locks,
   secure hashing (OpenSSL), and memory management. */

typedef struct {
    char key[64];
    char value[1024];
    int active;
} StoreEntry;

typedef struct {
    char token[128];
    time_t timestamp;
    int active;
} SessionEntry;

StoreEntry store[MAX_STORE_SIZE];
SessionEntry sessions[MAX_CONNECTIONS];

void init_store() {
    for (int i = 0; i < MAX_STORE_SIZE; i++) {
        store[i].active = 0;
    }
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        sessions[i].active = 0;
    }
}

int authenticate(const char* username, const char* password) {
    return (strcmp(username, "admin") == 0 && strcmp(password, "admin123") == 0);
}

void generate_token(char* token) {
    srand(time(NULL));
    sprintf(token, "TOKEN_%d_%d", rand(), (int)time(NULL));
}

char* create_session(const char* username) {
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (!sessions[i].active) {
            generate_token(sessions[i].token);
            sessions[i].timestamp = time(NULL);
            sessions[i].active = 1;
            return sessions[i].token;
        }
    }
    return NULL;
}

int validate_session(const char* token) {
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (sessions[i].active && strcmp(sessions[i].token, token) == 0) {
            if (time(NULL) - sessions[i].timestamp > SESSION_TIMEOUT) {
                sessions[i].active = 0;
                return 0;
            }
            return 1;
        }
    }
    return 0;
}

int is_valid_key(const char* key) {
    if (key == NULL || strlen(key) == 0 || strlen(key) > 50) return 0;
    for (int i = 0; key[i]; i++) {
        if (!((key[i] >= 'a' && key[i] <= 'z') ||
              (key[i] >= 'A' && key[i] <= 'Z') ||
              (key[i] >= '0' && key[i] <= '9') ||
              key[i] == '_' || key[i] == '-')) {
            return 0;
        }
    }
    return 1;
}

char* put_object(const char* key, const char* value, const char* token) {
    static char response[256];
    if (!validate_session(token)) {
        strcpy(response, "ERROR: Invalid session");
        return response;
    }
    if (!is_valid_key(key)) {
        strcpy(response, "ERROR: Invalid key format");
        return response;
    }
    
    for (int i = 0; i < MAX_STORE_SIZE; i++) {
        if (!store[i].active || strcmp(store[i].key, key) == 0) {
            strncpy(store[i].key, key, sizeof(store[i].key) - 1);
            strncpy(store[i].value, value, sizeof(store[i].value) - 1);
            store[i].active = 1;
            strcpy(response, "SUCCESS: Object stored");
            return response;
        }
    }
    strcpy(response, "ERROR: Store full");
    return response;
}

char* get_object(const char* key, const char* token) {
    static char response[1024];
    if (!validate_session(token)) {
        strcpy(response, "ERROR: Invalid session");
        return response;
    }
    if (!is_valid_key(key)) {
        strcpy(response, "ERROR: Invalid key format");
        return response;
    }
    
    for (int i = 0; i < MAX_STORE_SIZE; i++) {
        if (store[i].active && strcmp(store[i].key, key) == 0) {
            strcpy(response, store[i].value);
            return response;
        }
    }
    strcpy(response, "ERROR: Key not found");
    return response;
}

int main() {
    init_store();
    
    printf("=== Test Case 1: Authentication ===\\n");
    if (authenticate("admin", "admin123")) {
        char* token = create_session("admin");
        printf("SUCCESS: Authenticated. Token: %s\\n", token);
        
        printf("\\n=== Test Case 2: Put Object ===\\n");
        printf("%s\\n", put_object("key1", "value1", token));
        
        printf("\\n=== Test Case 3: Get Object ===\\n");
        printf("%s\\n", get_object("key1", token));
        
        printf("\\n=== Test Case 4: Put Another Object ===\\n");
        printf("%s\\n", put_object("key2", "value2", token));
        
        printf("\\n=== Test Case 5: Invalid Key Test ===\\n");
        printf("%s\\n", put_object("invalid@key", "value", token));
    }
    
    return 0;
}
