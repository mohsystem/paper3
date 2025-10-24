
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <openssl/sha.h>
#include <openssl/rand.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
    #define close closesocket
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <pthread.h>
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
#endif

#define PORT 8080
#define MAX_LOGIN_ATTEMPTS 3
#define LOCKOUT_TIME 300
#define MAX_USERS 100
#define USERS_FILE "users.dat"

typedef struct {
    char username[51];
    int attempts;
    time_t lockout_time;
} LoginAttempt;

static LoginAttempt login_attempts[MAX_USERS];
static int attempt_count = 0;

/* Base64 encoding table */
static const char base64_table[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void base64_encode(const unsigned char* input, size_t length, char* output) {
    size_t i, j;
    for (i = 0, j = 0; i < length; i += 3, j += 4) {
        unsigned int n = (i < length ? input[i] << 16 : 0) |
                        (i + 1 < length ? input[i + 1] << 8 : 0) |
                        (i + 2 < length ? input[i + 2] : 0);
        
        output[j] = base64_table[(n >> 18) & 0x3F];
        output[j + 1] = base64_table[(n >> 12) & 0x3F];
        output[j + 2] = (i + 1 < length) ? base64_table[(n >> 6) & 0x3F] : '=';
        output[j + 3] = (i + 2 < length) ? base64_table[n & 0x3F] : '=';
    }
    output[j] = '\\0';
}

void generate_salt(char* salt) {
    unsigned char random_bytes[16];
    RAND_bytes(random_bytes, sizeof(random_bytes));
    base64_encode(random_bytes, sizeof(random_bytes), salt);
}

void hash_password(const char* password, const char* salt, char* output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, salt, strlen(salt));
    SHA256_Update(&sha256, password, strlen(password));
    SHA256_Final(hash, &sha256);
    base64_encode(hash, SHA256_DIGEST_LENGTH, output);
}

void initialize_users_file() {
    FILE* file = fopen(USERS_FILE, "w");
    if (!file) return;
    
    const char* users[][2] = {
        {"user1", "password1"},
        {"user2", "password2"},
        {"admin", "admin123"},
        {"testuser", "test123"},
        {"alice", "alice456"}
    };
    
    int num_users = sizeof(users) / sizeof(users[0]);
    for (int i = 0; i < num_users; i++) {
        char salt[32];
        char hash[64];
        generate_salt(salt);
        hash_password(users[i][1], salt, hash);
        fprintf(file, "%s:%s:%s\\n", users[i][0], salt, hash);
    }
    
    fclose(file);
}

int is_valid_username(const char* username) {
    if (!username || strlen(username) == 0 || strlen(username) > 50)
        return 0;
    
    for (size_t i = 0; i < strlen(username); i++) {
        if (!isalnum(username[i]) && username[i] != '_')
            return 0;
    }
    return 1;
}

void sanitize_input(const char* input, char* output) {
    size_t j = 0;
    for (size_t i = 0; i < strlen(input) && j < 50; i++) {
        if (isalnum(input[i]) || input[i] == '_') {
            output[j++] = input[i];
        }
    }
    output[j] = '\\0';
}

int is_account_locked(const char* username) {
    time_t current_time = time(NULL);
    for (int i = 0; i < attempt_count; i++) {
        if (strcmp(login_attempts[i].username, username) == 0) {
            if (current_time < login_attempts[i].lockout_time) {
                return 1;
            } else if (login_attempts[i].lockout_time > 0) {
                login_attempts[i].attempts = 0;
                login_attempts[i].lockout_time = 0;
            }
        }
    }
    return 0;
}

void increment_login_attempts(const char* username) {
    int found = 0;
    for (int i = 0; i < attempt_count; i++) {
        if (strcmp(login_attempts[i].username, username) == 0) {
            login_attempts[i].attempts++;
            if (login_attempts[i].attempts >= MAX_LOGIN_ATTEMPTS) {
                login_attempts[i].lockout_time = time(NULL) + LOCKOUT_TIME;
            }
            found = 1;
            break;
        }
    }
    
    if (!found && attempt_count < MAX_USERS) {
        strncpy(login_attempts[attempt_count].username, username, 50);
        login_attempts[attempt_count].attempts = 1;
        login_attempts[attempt_count].lockout_time = 0;
        attempt_count++;
    }
}

int authenticate_user(const char* username, const char* password) {
    FILE* file = fopen(USERS_FILE, "r");
    if (!file) {
        initialize_users_file();
        file = fopen(USERS_FILE, "r");
    }
    
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char stored_username[51], stored_salt[32], stored_hash[64];
        if (sscanf(line, "%50[^:]:%31[^:]:%63s", stored_username, stored_salt, stored_hash) == 3) {
            if (strcmp(stored_username, username) == 0) {
                char computed_hash[64];
                hash_password(password, stored_salt, computed_hash);
                fclose(file);
                return strcmp(computed_hash, stored_hash) == 0;
            }
        }
    }
    
    fclose(file);
    return 0;
}

void process_request(const char* request, char* response) {
    if (!request || strlen(request) > 1000) {
        strcpy(response, "ERROR:Invalid request format");
        return;
    }
    
    char req_copy[1024];
    strncpy(req_copy, request, sizeof(req_copy) - 1);
    req_copy[sizeof(req_copy) - 1] = '\\0';
    
    char* command = strtok(req_copy, ":");
    char* username_raw = strtok(NULL, ":");
    char* password = strtok(NULL, ":");
    
    if (!command || !username_raw || !password || strcmp(command, "LOGIN") != 0) {
        strcpy(response, "ERROR:Invalid request format");
        return;
    }
    
    char username[51];
    sanitize_input(username_raw, username);
    
    if (!is_valid_username(username)) {
        strcpy(response, "ERROR:Invalid username format");
        return;
    }
    
    if (is_account_locked(username)) {
        strcpy(response, "ERROR:Account temporarily locked");
        return;
    }
    
    if (authenticate_user(username, password)) {
        for (int i = 0; i < attempt_count; i++) {
            if (strcmp(login_attempts[i].username, username) == 0) {
                login_attempts[i].attempts = 0;
                login_attempts[i].lockout_time = 0;
                break;
            }
        }
        strcpy(response, "SUCCESS:Login successful");
    } else {
        increment_login_attempts(username);
        strcpy(response, "ERROR:Invalid credentials");
    }
}

#ifdef _WIN32
DWORD WINAPI handle_client(LPVOID arg) {
    SOCKET client_socket = *(SOCKET*)arg;
    free(arg);
#else
void* handle_client(void* arg) {
    int client_socket = *(int*)arg;
    free(arg);
#endif
    
    char buffer[1024] = {0};
    int bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    
    char response[256];
    if (bytes_read > 0) {
        buffer[bytes_read] = '\\0';
        /* Remove newline characters */
        for (int i = 0; i < bytes_read; i++) {
            if (buffer[i] == '\\n' || buffer[i] == '\\r') {
                buffer[i] = '\\0';
                break;
            }
        }
        process_request(buffer, response);
    } else {
        strcpy(response, "ERROR:Invalid request");
    }
    
    strcat(response, "\\n");
    send(client_socket, response, strlen(response), 0);
    close(client_socket);
    
#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}

void start_server() {
#ifdef _WIN32
    WSADATA wsa_data;
    WSAStartup(MAKEWORD(2, 2), &wsa_data);
#endif
    
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        printf("Socket creation failed\\n");
        return;
    }
    
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Bind failed\\n");
        close(server_socket);
        return;
    }
    
    if (listen(server_socket, 5) == SOCKET_ERROR) {
        printf("Listen failed\\n");
        close(server_socket);
        return;
    }
    
    printf("Server started on port %d\\n", PORT);
    
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        SOCKET client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        
        if (client_socket != INVALID_SOCKET) {
            SOCKET* socket_ptr = malloc(sizeof(SOCKET));
            *socket_ptr = client_socket;
            
#ifdef _WIN32
            CreateThread(NULL, 0, handle_client, socket_ptr, 0, NULL);
#else
            pthread_t thread_id;
            pthread_create(&thread_id, NULL, handle_client, socket_ptr);
            pthread_detach(thread_id);
#endif
        }
    }
    
    close(server_socket);
#ifdef _WIN32
    WSACleanup();
#endif
}

char* client_login(const char* username, const char* password) {
    static char response[256];
    
    if (!username || !password || strlen(username) == 0 || strlen(password) == 0 ||
        strchr(username, '\\n') || strchr(password, '\\n')) {
        strcpy(response, "ERROR:Invalid input format");
        return response;
    }
    
    if (strlen(username) > 50 || strlen(password) > 100) {
        strcpy(response, "ERROR:Input too long");
        return response;
    }
    
    char sanitized_username[51];
    sanitize_input(username, sanitized_username);
    
    char request[256];
    snprintf(request, sizeof(request), "LOGIN:%s:%s\\n", sanitized_username, password);
    
#ifdef _WIN32
    WSADATA wsa_data;
    WSAStartup(MAKEWORD(2, 2), &wsa_data);
#endif
    
    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) {
        strcpy(response, "ERROR:Socket creation failed");
        return response;
    }
    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    
#ifdef _WIN32
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
#else
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
#endif
    
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        close(client_socket);
        strcpy(response, "ERROR:Connection failed");
        return response;
    }
    
    send(client_socket, request, strlen(request), 0);
    
    char buffer[1024] = {0};
    int bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    
    close(client_socket);
    
    if (bytes_read > 0) {
        buffer[bytes_read] = '\\0';
        for (int i = 0; i < bytes_read; i++) {
            if (buffer[i] == '\\n' || buffer[i] == '\\r') {
                buffer[i] = '\\0';
                break;
            }
        }
        strncpy(response, buffer, sizeof(response) - 1);
        response[sizeof(response) - 1] = '\\0';
    } else {
        strcpy(response, "ERROR:No response from server");
    }
    
    return response;
}

#ifdef _WIN32
DWORD WINAPI server_thread(LPVOID arg) {
    start_server();
    return 0;
}
#else
void* server_thread(void* arg) {
    start_server();
    return NULL;
}
#endif

int main() {
    /* Start server in a separate thread */
#ifdef _WIN32
    CreateThread(NULL, 0, server_thread, NULL, 0, NULL);
    Sleep(1000);
#else
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, server_thread, NULL);
    pthread_detach(thread_id);
    sleep(1);
#endif
    
    /* Test cases */
    printf("=== Running Test Cases ===\\n\\n");
    
    /* Test case 1: Valid login */
    printf("Test 1 - Valid user login:\\n");
    char* response1 = client_login("user1", "password1");
    printf("Response: %s\\n\\n", response1);
    
    /* Test case 2: Invalid password */
    printf("Test 2 - Invalid password:\\n");
    char* response2 = client_login("user1", "wrongpassword");
    printf("Response: %s\\n\\n", response2);
    
    /* Test case 3: Non-existent user */
    printf("Test 3 - Non-existent user:\\n");
    char* response3 = client_login("nonexistent", "password");
    printf("Response: %s\\n\\n", response3);
    
    /* Test case 4: Valid admin login */
    printf("Test 4 - Admin login:\\n");
    char* response4 = client_login("admin", "admin123");
    printf("Response: %s\\n\\n", response4);
    
    /* Test case 5: Empty credentials */
    printf("Test 5 - Empty credentials:\\n");
    char* response5 = client_login("", "");
    printf("Response: %s\\n\\n", response5);
    
    /* Keep the program running */
#ifdef _WIN32
    Sleep(2000);
#else
    sleep(2);
#endif
    
    return 0;
}
