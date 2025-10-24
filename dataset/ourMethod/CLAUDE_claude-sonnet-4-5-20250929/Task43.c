
// C Secure Session Management Web Application
// Uses OpenSSL for cryptographic operations and simple socket server
// Compile: gcc -std=c11 -o session_app session_app.c -lssl -lcrypto -pthread

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <ctype.h>

#define SESSION_TIMEOUT 900
#define SESSION_ID_BYTES 32
#define SESSION_ID_HEX_LEN (SESSION_ID_BYTES * 2)
#define MAX_SESSIONS 1000
#define MAX_USER_ID 256
#define PORT 8080
#define BUFFER_SIZE 4096

// Session structure
typedef struct {
    char session_id[SESSION_ID_HEX_LEN + 1];
    char user_id[MAX_USER_ID + 1];
    time_t created_at;
    time_t last_accessed;
    int active;
} Session;

// Global session storage with mutex
static Session g_sessions[MAX_SESSIONS];
static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

// Securely clear memory (Rules#1)
static void secure_zero(void* ptr, size_t len) {
    if (ptr != NULL && len > 0) {
        OPENSSL_cleanse(ptr, len);
    }
}

// Generate cryptographically secure session ID (Rules#6, #7)
static int generate_session_id(char* out_buffer, size_t buffer_size) {
    unsigned char random_bytes[SESSION_ID_BYTES];
    
    // Validate output buffer (Rules#1, #16)
    if (out_buffer == NULL || buffer_size < (SESSION_ID_HEX_LEN + 1)) {
        return 0;
    }
    
    // Use cryptographically secure RNG
    if (RAND_bytes(random_bytes, SESSION_ID_BYTES) != 1) {
        return 0;
    }
    
    // Convert to hex string
    for (size_t i = 0; i < SESSION_ID_BYTES; i++) {
        snprintf(out_buffer + (i * 2), 3, "%02x", random_bytes[i]);
    }
    out_buffer[SESSION_ID_HEX_LEN] = '\\0';
    
    // Clear sensitive data (Rules#1)
    secure_zero(random_bytes, SESSION_ID_BYTES);
    
    return 1;
}

// Validate session ID format (Rules#1, #15)
static int validate_session_id(const char* session_id) {
    if (session_id == NULL) {
        return 0;
    }
    
    size_t len = strlen(session_id);
    if (len != SESSION_ID_HEX_LEN) {
        return 0;
    }
    
    // Check all characters are hex digits
    for (size_t i = 0; i < len; i++) {
        if (!isxdigit((unsigned char)session_id[i])) {
            return 0;
        }
    }
    
    return 1;
}

// Create new session (Rules#1, #15)
static int create_session(const char* user_id, char* out_session_id, size_t out_size) {
    if (user_id == NULL || out_session_id == NULL || out_size < (SESSION_ID_HEX_LEN + 1)) {
        return 0;
    }
    
    // Validate user_id (Rules#1, #15)
    size_t user_len = strlen(user_id);
    if (user_len == 0 || user_len > MAX_USER_ID) {
        return 0;
    }
    
    pthread_mutex_lock(&g_mutex);
    
    // Find free slot
    int slot = -1;
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (!g_sessions[i].active) {
            slot = i;
            break;
        }
    }
    
    if (slot == -1) {
        pthread_mutex_unlock(&g_mutex);
        return 0;
    }
    
    // Generate session ID
    if (!generate_session_id(g_sessions[slot].session_id, sizeof(g_sessions[slot].session_id))) {
        pthread_mutex_unlock(&g_mutex);
        return 0;
    }
    
    // Copy user_id safely (Rules#1, #16)
    strncpy(g_sessions[slot].user_id, user_id, MAX_USER_ID);
    g_sessions[slot].user_id[MAX_USER_ID] = '\\0';
    
    g_sessions[slot].created_at = time(NULL);
    g_sessions[slot].last_accessed = g_sessions[slot].created_at;
    g_sessions[slot].active = 1;
    
    // Copy session ID to output
    strncpy(out_session_id, g_sessions[slot].session_id, out_size - 1);
    out_session_id[out_size - 1] = '\\0';
    
    pthread_mutex_unlock(&g_mutex);
    return 1;
}

// Get session by ID
static Session* get_session(const char* session_id) {
    if (!validate_session_id(session_id)) {
        return NULL;
    }
    
    pthread_mutex_lock(&g_mutex);
    
    time_t now = time(NULL);
    Session* result = NULL;
    
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (g_sessions[i].active && 
            strcmp(g_sessions[i].session_id, session_id) == 0) {
            
            // Check expiration (Rules#1)
            if (now < g_sessions[i].last_accessed || 
                (now - g_sessions[i].last_accessed) > SESSION_TIMEOUT) {
                // Expired - clear and mark inactive
                secure_zero(g_sessions[i].user_id, sizeof(g_sessions[i].user_id));
                secure_zero(g_sessions[i].session_id, sizeof(g_sessions[i].session_id));
                g_sessions[i].active = 0;
                result = NULL;
            } else {
                g_sessions[i].last_accessed = now;
                result = &g_sessions[i];
            }
            break;
        }
    }
    
    pthread_mutex_unlock(&g_mutex);
    return result;
}

// Terminate session (Rules#1)
static int terminate_session(const char* session_id) {
    if (!validate_session_id(session_id)) {
        return 0;
    }
    
    pthread_mutex_lock(&g_mutex);
    
    int found = 0;
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (g_sessions[i].active && 
            strcmp(g_sessions[i].session_id, session_id) == 0) {
            
            // Securely clear session data (Rules#1)
            secure_zero(g_sessions[i].user_id, sizeof(g_sessions[i].user_id));
            secure_zero(g_sessions[i].session_id, sizeof(g_sessions[i].session_id));
            g_sessions[i].active = 0;
            found = 1;
            break;
        }
    }
    
    pthread_mutex_unlock(&g_mutex);
    return found;
}

// Extract parameter from query string
static int extract_param(const char* query, const char* param_name, 
                        char* out_value, size_t out_size) {
    if (query == NULL || param_name == NULL || out_value == NULL || out_size == 0) {
        return 0;
    }
    
    size_t name_len = strlen(param_name);
    const char* start = strstr(query, param_name);
    
    if (start == NULL) {
        return 0;
    }
    
    start += name_len;
    if (*start != '=') {
        return 0;
    }
    start++;
    
    const char* end = strchr(start, '&');
    size_t value_len = end ? (size_t)(end - start) : strlen(start);
    
    // Validate length (Rules#1, #16)
    if (value_len == 0 || value_len >= out_size) {
        return 0;
    }
    
    strncpy(out_value, start, value_len);
    out_value[value_len] = '\\0';
    
    return 1;
}

// Extract cookie value
static int extract_cookie(const char* cookie_header, const char* name,
                         char* out_value, size_t out_size) {
    if (cookie_header == NULL || name == NULL || out_value == NULL || out_size == 0) {
        return 0;
    }
    
    char search[128];
    snprintf(search, sizeof(search), "%s=", name);
    
    const char* start = strstr(cookie_header, search);
    if (start == NULL) {
        return 0;
    }
    
    start += strlen(search);
    const char* end = strchr(start, ';');
    size_t value_len = end ? (size_t)(end - start) : strlen(start);
    
    // Validate length (Rules#1, #16)
    if (value_len == 0 || value_len >= out_size) {
        return 0;
    }
    
    strncpy(out_value, start, value_len);
    out_value[value_len] = '\\0';
    
    return 1;
}

// Handle HTTP request
static void handle_request(int client_socket) {
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    
    // Initialize buffers (Rules#1)
    memset(buffer, 0, sizeof(buffer));
    memset(response, 0, sizeof(response));
    
    // Read request with bounds check (Rules#1, #16)
    ssize_t bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_read <= 0) {
        close(client_socket);
        return;
    }
    buffer[bytes_read] = '\\0';
    
    // Parse request line
    char method[16] = {0};
    char path[256] = {0};
    
    if (sscanf(buffer, "%15s %255s", method, path) != 2) {
        const char* error_resp = "HTTP/1.1 400 Bad Request\\r\\n\\r\\n";
        send(client_socket, error_resp, strlen(error_resp), 0);
        close(client_socket);
        return;
    }
    
    // Route: Create session
    if (strcmp(path, "/create") == 0 || strncmp(path, "/create?", 8) == 0) {
        char user_id[MAX_USER_ID + 1] = {0};
        
        if (!extract_param(path, "user", user_id, sizeof(user_id))) {
            snprintf(response, sizeof(response),
                    "HTTP/1.1 400 Bad Request\\r\\n"
                    "Content-Type: application/json\\r\\n\\r\\n"
                    "{\\"error\\":\\"Invalid user parameter\\"}");
        } else {
            char session_id[SESSION_ID_HEX_LEN + 1] = {0};
            
            if (create_session(user_id, session_id, sizeof(session_id))) {
                snprintf(response, sizeof(response),
                        "HTTP/1.1 200 OK\\r\\n"
                        "Content-Type: application/json\\r\\n"
                        "Set-Cookie: session_id=%s; HttpOnly; Secure; SameSite=Strict; Max-Age=900\\r\\n\\r\\n"
                        "{\\"status\\":\\"Session created\\"}",
                        session_id);
            } else {
                snprintf(response, sizeof(response),
                        "HTTP/1.1 500 Internal Server Error\\r\\n"
                        "Content-Type: application/json\\r\\n\\r\\n"
                        "{\\"error\\":\\"Failed to create session\\"}");
            }
        }
    }
    // Route: Check session
    else if (strcmp(path, "/check") == 0) {
        char* cookie_line = strstr(buffer, "Cookie:");
        char session_id[SESSION_ID_HEX_LEN + 1] = {0};
        
        if (cookie_line && extract_cookie(cookie_line, "session_id", 
                                         session_id, sizeof(session_id))) {
            Session* session = get_session(session_id);
            
            if (session != NULL) {
                snprintf(response, sizeof(response),
                        "HTTP/1.1 200 OK\\r\\n"
                        "Content-Type: application/json\\r\\n\\r\\n"
                        "{\\"status\\":\\"Valid session\\",\\"user\\":\\"%s\\"}",
                        session->user_id);
            } else {
                snprintf(response, sizeof(response),
                        "HTTP/1.1 401 Unauthorized\\r\\n"
                        "Content-Type: application/json\\r\\n\\r\\n"
                        "{\\"status\\":\\"No valid session\\"}");
            }
        } else {
            snprintf(response, sizeof(response),
                    "HTTP/1.1 401 Unauthorized\\r\\n"
                    "Content-Type: application/json\\r\\n\\r\\n"
                    "{\\"status\\":\\"No valid session\\"}");
        }
    }
    // Route: Logout
    else if (strcmp(path, "/logout") == 0) {
        char* cookie_line = strstr(buffer, "Cookie:");
        char session_id[SESSION_ID_HEX_LEN + 1] = {0};
        
        if (cookie_line && extract_cookie(cookie_line, "session_id",
                                         session_id, sizeof(session_id))) {
            if (terminate_session(session_id)) {
                snprintf(response, sizeof(response),
                        "HTTP/1.1 200 OK\\r\\n"
                        "Content-Type: application/json\\r\\n"
                        "Set-Cookie: session_id=; HttpOnly; Secure; SameSite=Strict; Max-Age=0\\r\\n\\r\\n"
                        "{\\"status\\":\\"Session terminated\\"}");
            } else {
                snprintf(response, sizeof(response),
                        "HTTP/1.1 400 Bad Request\\r\\n"
                        "Content-Type: application/json\\r\\n\\r\\n"
                        "{\\"error\\":\\"No session to terminate\\"}");
            }
        } else {
            snprintf(response, sizeof(response),
                    "HTTP/1.1 400 Bad Request\\r\\n"
                    "Content-Type: application/json\\r\\n\\r\\n"
                    "{\\"error\\":\\"No session cookie\\"}");
        }
    }
    else {
        snprintf(response, sizeof(response),
                "HTTP/1.1 404 Not Found\\r\\n"
                "Content-Type: application/json\\r\\n\\r\\n"
                "{\\"error\\":\\"Not found\\"}");
    }
    
    send(client_socket, response, strlen(response), 0);
    close(client_socket);
}

// Thread function for handling client
static void* client_thread(void* arg) {
    int client_socket = *((int*)arg);
    free(arg);
    handle_request(client_socket);
    return NULL;
}

int main(void) {
    int server_socket;
    struct sockaddr_in server_addr;
    
    // Initialize session storage (Rules#1)
    memset(g_sessions, 0, sizeof(g_sessions));
    
    // Initialize OpenSSL
    OpenSSL_add_all_algorithms();
    
    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        return 1;
    }
    
    // Set socket options
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Setsockopt failed");
        close(server_socket);
        return 1;
    }
    
    // Bind socket
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        return 1;
    }
    
    // Listen
    if (listen(server_socket, 10) < 0) {
        perror("Listen failed");
        close(server_socket);
        return 1;
    }
    
    printf("Session Management Server running on port %d\\n", PORT);
    printf("\\nTest with curl:\\n");
    printf("1. Create: curl 'http://localhost:%d/create?user=testuser' -c cookies.txt\\n", PORT);
    printf("2. Check: curl http://localhost:%d/check -b cookies.txt\\n", PORT);
    printf("3. Logout: curl http://localhost:%d/logout -b cookies.txt\\n", PORT);
    printf("4. Check again: curl http://localhost:%d/check -b cookies.txt\\n", PORT);
    printf("5. Invalid: curl http://localhost:%d/invalid\\n", PORT);
    
    // Accept connections
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int* client_socket = malloc(sizeof(int));
        if (client_socket == NULL) {
            continue;
        }
        
        *client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (*client_socket < 0) {
            free(client_socket);
            continue;
        }
        
        // Create thread to handle request (Rules#17)
        pthread_t thread;
        if (pthread_create(&thread, NULL, client_thread, client_socket) != 0) {
            close(*client_socket);
            free(client_socket);
        } else {
            pthread_detach(thread);
        }
    }
    
    close(server_socket);
    EVP_cleanup();
    pthread_mutex_destroy(&g_mutex);
    
    return 0;
}
