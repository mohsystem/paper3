
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/rand.h>
#include <ctype.h>

#define MAX_SESSIONS 100
#define SESSION_ID_LEN 65
#define MAX_STRING_LEN 256
#define BUFFER_SIZE 4096

typedef struct {
    char session_id[SESSION_ID_LEN];
    char username[MAX_STRING_LEN];
    char email[MAX_STRING_LEN];
    char ssn_last4[5];
} UserSession;

UserSession sessions[MAX_SESSIONS];
int session_count = 0;

/* Sanitize output to prevent XSS - returns length written */
size_t sanitize_output(const char* input, char* output, size_t output_size) {
    if (input == NULL || output == NULL || output_size == 0) {
        return 0;
    }
    
    size_t out_pos = 0;
    const char* in_ptr = input;
    
    while (*in_ptr != '\\0' && out_pos < output_size - 1) {
        const char* replacement = NULL;
        size_t rep_len = 0;
        
        switch (*in_ptr) {
            case '&': replacement = "&amp;"; rep_len = 5; break;
            case '<': replacement = "&lt;"; rep_len = 4; break;
            case '>': replacement = "&gt;"; rep_len = 4; break;
            case '"': replacement = "&quot;"; rep_len = 6; break;
            case '\\'': replacement = "&#x27;"; rep_len = 6; break;
            case '/': replacement = "&#x2F;"; rep_len = 6; break;
            default:
                if (out_pos < output_size - 1) {
                    output[out_pos++] = *in_ptr;
                }
                in_ptr++;
                continue;
        }
        
        if (replacement && out_pos + rep_len < output_size) {
            memcpy(output + out_pos, replacement, rep_len);
            out_pos += rep_len;
        }
        in_ptr++;
    }
    
    output[out_pos] = '\\0';
    return out_pos;
}

/* Validate SSN last 4 digits */
int validate_ssn_last4(const char* ssn, char* output) {
    if (ssn == NULL || output == NULL) {
        return 0;
    }
    
    if (strlen(ssn) != 4) {
        strcpy(output, "****");
        return 0;
    }
    
    for (int i = 0; i < 4; i++) {
        if (!isdigit((unsigned char)ssn[i])) {
            strcpy(output, "****");
            return 0;
        }
    }
    
    strncpy(output, ssn, 5);
    output[4] = '\\0';
    return 1;
}

/* Generate cryptographically secure session ID */
int generate_session_id(char* output) {
    unsigned char buffer[32];
    
    if (RAND_bytes(buffer, sizeof(buffer)) != 1) {
        return 0;
    }
    
    for (size_t i = 0; i < sizeof(buffer); i++) {
        snprintf(output + (i * 2), 3, "%02x", buffer[i]);
    }
    output[64] = '\\0';
    
    return 1;
}

/* Extract cookie value from headers */
int get_cookie(const char* headers, const char* name, char* value, size_t value_size) {
    if (headers == NULL || name == NULL || value == NULL || value_size == 0) {
        return 0;
    }
    
    const char* cookie_start = strstr(headers, "Cookie:");
    if (cookie_start == NULL) {
        return 0;
    }
    
    cookie_start += 7;
    const char* line_end = strstr(cookie_start, "\\r\\n");
    if (line_end == NULL) {
        line_end = cookie_start + strlen(cookie_start);
    }
    
    size_t name_len = strlen(name);
    const char* pos = cookie_start;
    
    while (pos < line_end) {
        while (pos < line_end && isspace((unsigned char)*pos)) pos++;
        
        if (strncmp(pos, name, name_len) == 0 && pos[name_len] == '=') {
            pos += name_len + 1;
            size_t i = 0;
            while (pos < line_end && *pos != ';' && i < value_size - 1) {
                value[i++] = *pos++;
            }
            value[i] = '\\0';
            return 1;
        }
        
        while (pos < line_end && *pos != ';') pos++;
        if (pos < line_end) pos++;
    }
    
    return 0;
}

/* Find session by ID */
UserSession* find_session(const char* session_id) {
    for (int i = 0; i < session_count; i++) {
        if (strcmp(sessions[i].session_id, session_id) == 0) {
            return &sessions[i];
        }
    }
    return NULL;
}

/* Handle HTTP request */
void handle_request(int client_socket) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));
    
    ssize_t bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);
    if (bytes_read <= 0) {
        close(client_socket);
        return;
    }
    
    buffer[bytes_read] = '\\0';
    
    if (strncmp(buffer, "GET /info", 9) != 0) {
        const char* response = "HTTP/1.1 404 Not Found\\r\\n"
                              "Content-Type: text/plain\\r\\n"
                              "Content-Length: 9\\r\\n\\r\\n"
                              "Not Found";
        write(client_socket, response, strlen(response));
        close(client_socket);
        return;
    }
    
    char session_id[SESSION_ID_LEN];
    if (!get_cookie(buffer, "sessionId", session_id, sizeof(session_id))) {
        const char* response = "HTTP/1.1 401 Unauthorized\\r\\n"
                              "Content-Type: text/plain\\r\\n"
                              "Content-Length: 12\\r\\n\\r\\n"
                              "Unauthorized";
        write(client_socket, response, strlen(response));
        close(client_socket);
        return;
    }
    
    UserSession* user = find_session(session_id);
    if (user == NULL) {
        const char* response = "HTTP/1.1 401 Unauthorized\\r\\n"
                              "Content-Type: text/plain\\r\\n"
                              "Content-Length: 12\\r\\n\\r\\n"
                              "Unauthorized";
        write(client_socket, response, strlen(response));
        close(client_socket);
        return;
    }
    
    char safe_username[MAX_STRING_LEN * 6];
    char safe_email[MAX_STRING_LEN * 6];
    char safe_ssn[MAX_STRING_LEN * 6];
    
    sanitize_output(user->username, safe_username, sizeof(safe_username));
    sanitize_output(user->email, safe_email, sizeof(safe_email));
    sanitize_output(user->ssn_last4, safe_ssn, sizeof(safe_ssn));
    
    char html[BUFFER_SIZE];
    int html_len = snprintf(html, sizeof(html),
        "<!DOCTYPE html>\\n<html>\\n<head>\\n"
        "<meta charset=\\"UTF-8\\">\\n"
        "<meta http-equiv=\\"Content-Security-Policy\\" content=\\"default-src 'self'\\">\\n"
        "<title>User Info</title>\\n</head>\\n<body>\\n"
        "<h1>User Information</h1>\\n"
        "<p>Username: %s</p>\\n"
        "<p>Email: %s</p>\\n"
        "<p>SSN (Last 4): %s</p>\\n"
        "</body>\\n</html>",
        safe_username, safe_email, safe_ssn);
    
    char response[BUFFER_SIZE];
    int resp_len = snprintf(response, sizeof(response),
        "HTTP/1.1 200 OK\\r\\n"
        "Content-Type: text/html; charset=UTF-8\\r\\n"
        "X-Content-Type-Options: nosniff\\r\\n"
        "X-Frame-Options: DENY\\r\\n"
        "X-XSS-Protection: 1; mode=block\\r\\n"
        "Content-Length: %d\\r\\n\\r\\n%s",
        html_len, html);
    
    write(client_socket, response, resp_len);
    close(client_socket);
}

int main(void) {
    OPENSSL_init_crypto(0, NULL);
    
    generate_session_id(sessions[0].session_id);
    strncpy(sessions[0].username, "john_doe", MAX_STRING_LEN - 1);
    strncpy(sessions[0].email, "john@example.com", MAX_STRING_LEN - 1);
    validate_ssn_last4("1234", sessions[0].ssn_last4);
    
    generate_session_id(sessions[1].session_id);
    strncpy(sessions[1].username, "jane_smith", MAX_STRING_LEN - 1);
    strncpy(sessions[1].email, "jane@example.com", MAX_STRING_LEN - 1);
    validate_ssn_last4("5678", sessions[1].ssn_last4);
    
    generate_session_id(sessions[2].session_id);
    strncpy(sessions[2].username, "bob_jones", MAX_STRING_LEN - 1);
    strncpy(sessions[2].email, "bob@example.com", MAX_STRING_LEN - 1);
    validate_ssn_last4("9012", sessions[2].ssn_last4);
    
    generate_session_id(sessions[3].session_id);
    strncpy(sessions[3].username, "alice<script>", MAX_STRING_LEN - 1);
    strncpy(sessions[3].email, "alice@test.com", MAX_STRING_LEN - 1);
    validate_ssn_last4("3456", sessions[3].ssn_last4);
    
    generate_session_id(sessions[4].session_id);
    strncpy(sessions[4].username, "charlie", MAX_STRING_LEN - 1);
    strncpy(sessions[4].email, "charlie@example.com", MAX_STRING_LEN - 1);
    validate_ssn_last4("7890", sessions[4].ssn_last4);
    
    session_count = 5;
    
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        return 1;
    }
    
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(server_socket);
        return 1;
    }
    
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8000);
    
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        return 1;
    }
    
    if (listen(server_socket, 10) < 0) {
        perror("Listen failed");
        close(server_socket);
        return 1;
    }
    
    printf("Server started on port 8000\\n");
    printf("Test sessions created:\\n");
    for (int i = 0; i < session_count; i++) {
        printf("Session %d: %s\\n", i + 1, sessions[i].session_id);
    }
    printf("\\nAccess /info with Cookie: sessionId=<session_id>\\n");
    
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        
        if (client_socket >= 0) {
            handle_request(client_socket);
        }
    }
    
    close(server_socket);
    return 0;
}
