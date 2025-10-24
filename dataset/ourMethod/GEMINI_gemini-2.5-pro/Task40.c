#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

// Note: This is a simplified, single-threaded, sequential-handling web server
// for demonstration purposes only. It is not robust for production use.
// It lacks proper error handling, timeout management, and concurrency.
// This example is designed to run on a POSIX-compliant system (like Linux or macOS).

#define PORT 8080
#define BUFFER_SIZE 4096
#define MAX_SESSIONS 100
#define CSRF_TOKEN_BYTES 32
#define CSRF_TOKEN_HEX_LEN (CSRF_TOKEN_BYTES * 2)
#define SESSION_ID_BYTES 16
#define SESSION_ID_HEX_LEN (SESSION_ID_BYTES * 2)
#define MAX_EMAIL_LEN 128

typedef struct {
    char session_id[SESSION_ID_HEX_LEN + 1];
    char csrf_token[CSRF_TOKEN_HEX_LEN + 1];
    char email[MAX_EMAIL_LEN];
    bool is_active;
} Session;

// Simple in-memory session store. In a real application, use a proper session manager.
Session session_store[MAX_SESSIONS];

void initialize_sessions() {
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        session_store[i].is_active = false;
        memset(session_store[i].session_id, 0, sizeof(session_store[i].session_id));
        memset(session_store[i].csrf_token, 0, sizeof(session_store[i].csrf_token));
        strncpy(session_store[i].email, "user@example.com", MAX_EMAIL_LEN - 1);
    }
}

// Generates cryptographically secure random bytes and hex-encodes them.
// Returns 0 on success, -1 on failure.
int generate_secure_token(char *hex_buffer, size_t buffer_len, size_t byte_length) {
    if (buffer_len < byte_length * 2 + 1) {
        return -1; // Buffer too small
    }
    unsigned char *random_bytes = malloc(byte_length);
    if (!random_bytes) {
        return -1;
    }

    int fd = open("/dev/urandom", O_RDONLY);
    if (fd == -1) {
        perror("open /dev/urandom");
        free(random_bytes);
        return -1;
    }

    ssize_t bytes_read = read(fd, random_bytes, byte_length);
    close(fd);

    if (bytes_read != (ssize_t)byte_length) {
        fprintf(stderr, "Failed to read enough bytes from /dev/urandom\n");
        free(random_bytes);
        return -1;
    }

    for (size_t i = 0; i < byte_length; ++i) {
        sprintf(hex_buffer + (i * 2), "%02x", random_bytes[i]);
    }
    hex_buffer[byte_length * 2] = '\0';
    
    free(random_bytes);
    return 0;
}

// Compares two strings in a way that resists timing attacks.
bool constant_time_compare(const char* a, const char* b) {
    if (a == NULL || b == NULL) return false;
    size_t len_a = strlen(a);
    size_t len_b = strlen(b);

    if (len_a != len_b) {
        return false;
    }

    int result = 0;
    for (size_t i = 0; i < len_a; ++i) {
        result |= a[i] ^ b[i];
    }
    return result == 0;
}

Session* find_or_create_session(const char* session_id) {
    Session* free_slot = NULL;
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (session_store[i].is_active) {
            if (strncmp(session_store[i].session_id, session_id, SESSION_ID_HEX_LEN) == 0) {
                return &session_store[i];
            }
        } else if (free_slot == NULL) {
            free_slot = &session_store[i];
        }
    }

    if (free_slot) {
        free_slot->is_active = true;
        if (generate_secure_token(free_slot->session_id, SESSION_ID_HEX_LEN + 1, SESSION_ID_BYTES) != 0) {
            free_slot->is_active = false;
            return NULL;
        }
        return free_slot;
    }

    return NULL; // No free slots
}


void handle_get_settings(int client_socket, Session* session, const char* session_id) {
    if (generate_secure_token(session->csrf_token, CSRF_TOKEN_HEX_LEN + 1, CSRF_TOKEN_BYTES) != 0) {
         // Handle error, maybe send 500
         return;
    }

    char content[1024];
    snprintf(content, sizeof(content),
             "<html><head><title>User Settings</title></head>"
             "<body><h1>Update User Settings</h1>"
             "<p>Current Email: %s</p>"
             "<form action=\"/settings\" method=\"POST\">"
             "<label for=\"email\">New Email:</label><br>"
             "<input type=\"email\" id=\"email\" name=\"email\"><br><br>"
             "<input type=\"hidden\" name=\"csrf_token\" value=\"%s\">"
             "<input type=\"submit\" value=\"Update\">"
             "</form></body></html>",
             session->email, session->csrf_token);
    
    char response[2048];
    snprintf(response, sizeof(response),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: text/html\r\n"
             "Content-Length: %zu\r\n"
             "Set-Cookie: session_id=%s; HttpOnly; Path=/; SameSite=Lax\r\n"
             "\r\n"
             "%s",
             strlen(content), session_id, content);

    send(client_socket, response, strlen(response), 0);
}

void handle_post_settings(int client_socket, Session* session, const char* request_body) {
    char submitted_token[CSRF_TOKEN_HEX_LEN + 2] = "";
    char new_email[MAX_EMAIL_LEN] = "";

    // Simple manual parsing of form data. A robust solution would be more complex.
    const char* token_ptr = strstr(request_body, "csrf_token=");
    if (token_ptr) {
        sscanf(token_ptr, "csrf_token=%128[^&]", submitted_token);
    }
    
    const char* email_ptr = strstr(request_body, "email=");
    if (email_ptr) {
        sscanf(email_ptr, "email=%128[^&]", new_email);
        // Simple URL decoding for '@'
        char* at_symbol = strstr(new_email, "%40");
        if(at_symbol) {
            *at_symbol = '@';
            memmove(at_symbol + 1, at_symbol + 3, strlen(at_symbol + 3) + 1);
        }
    }
    
    if (strlen(session->csrf_token) == 0 || !constant_time_compare(session->csrf_token, submitted_token)) {
        const char* response_body = "<html><body><h1>403 Forbidden</h1><p>Invalid CSRF token.</p></body></html>";
        char http_response[512];
        snprintf(http_response, sizeof(http_response),
                 "HTTP/1.1 403 Forbidden\r\nContent-Type: text/html\r\nContent-Length: %zu\r\n\r\n%s",
                 strlen(response_body), response_body);
        send(client_socket, http_response, strlen(http_response), 0);
        return;
    }
    
    // Invalidate used token
    memset(session->csrf_token, 0, sizeof(session->csrf_token));

    if (strlen(new_email) > 0) {
        // In a real app, validate and sanitize this input
        strncpy(session->email, new_email, MAX_EMAIL_LEN - 1);
        session->email[MAX_EMAIL_LEN - 1] = '\0';
    }

    char response_body[512];
    snprintf(response_body, sizeof(response_body),
             "<html><body><h1>Settings Updated</h1><p>New email: %s</p><a href=\"/settings\">Go back</a></body></html>", session->email);
    
    char http_response[1024];
    snprintf(http_response, sizeof(http_response),
             "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %zu\r\n\r\n%s",
             strlen(response_body), response_body);
    send(client_socket, http_response, strlen(http_response), 0);
}


void handle_connection(int client_socket) {
    char buffer[BUFFER_SIZE] = {0};
    ssize_t bytes_read = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_read <= 0) {
        close(client_socket);
        return;
    }
    buffer[bytes_read] = '\0'; // Ensure null termination

    char method[16], path[256];
    sscanf(buffer, "%15s %255s", method, path);

    char session_id[SESSION_ID_HEX_LEN + 1] = "";
    char* cookie_ptr = strstr(buffer, "Cookie: session_id=");
    if (cookie_ptr) {
        sscanf(cookie_ptr, "Cookie: session_id=%32[^;\r\n]", session_id);
    }
    
    Session* session = find_or_create_session(session_id);
    if (!session) {
        // Handle no session available error (500)
        const char* response = "HTTP/1.1 500 Internal Server Error\r\n\r\nNo session available.";
        send(client_socket, response, strlen(response), 0);
        close(client_socket);
        return;
    }

    if (strcmp(path, "/settings") == 0 && strcmp(method, "GET") == 0) {
        handle_get_settings(client_socket, session, session->session_id);
    } else if (strcmp(path, "/settings") == 0 && strcmp(method, "POST") == 0) {
        char* body_ptr = strstr(buffer, "\r\n\r\n");
        if (body_ptr) {
            body_ptr += 4;
            handle_post_settings(client_socket, session, body_ptr);
        }
    } else {
        const char* response_body = "<html><body><h1>404 Not Found</h1><p>Try <a href=\"/settings\">/settings</a></p></body></html>";
        char http_response[512];
        snprintf(http_response, sizeof(http_response),
                 "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: %zu\r\n\r\n%s",
                 strlen(response_body), response_body);
        send(client_socket, http_response, strlen(http_response), 0);
    }

    close(client_socket);
}


int main() {
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);

    initialize_sessions();

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        return 1;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        return 1;
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        return 1;
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        return 1;
    }
    
    printf("Server listening on http://localhost:%d\n", PORT);
    printf("Access /settings to see the form.\n");
    printf("--- Test Cases ---\n");
    printf("1. Open a web browser and navigate to http://localhost:%d/settings\n", PORT);
    printf("2. Enter a new email and submit the form. You should see a success message.\n");
    printf("3. (Advanced) Use browser developer tools to inspect the form. Note the hidden 'csrf_token' value.\n");
    printf("4. Simulate a CSRF attack with a missing token using curl:\n");
    printf("   curl -X POST -d 'email=attacker@example.com' --cookie \"session_id=...\" http://localhost:%d/settings\n", PORT);
    printf("   (replace 'session_id=...' with the actual cookie from your browser). You should get a 403 Forbidden error.\n");
    printf("5. Simulate a CSRF attack with an incorrect token using curl:\n");
    printf("   curl -X POST -d 'email=attacker@example.com&csrf_token=invalid' --cookie \"session_id=...\" http://localhost:%d/settings\n", PORT);
    printf("   You should also get a 403 Forbidden error.\n");

    while (true) {
        int client_socket;
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
            perror("accept");
            continue;
        }
        handle_connection(client_socket);
    }

    close(server_fd);
    return 0;
}