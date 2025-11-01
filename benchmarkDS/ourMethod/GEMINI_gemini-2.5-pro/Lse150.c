#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>

// --- Constants ---
#define PORT 8081
#define BUFFER_SIZE 2048
#define MAX_USERNAME_LEN 32
#define MAX_MESSAGE_LEN 256
#define MAX_POST_BODY_SIZE (MAX_USERNAME_LEN + MAX_MESSAGE_LEN + 50)

// --- Mock Database Function ---
int insert_user_message_in_db(const char* username, const char* message) {
    if (username == NULL || message == NULL) {
        return 0;
    }
    printf("DATABASE: Inserting message for user '%s': '%s'\n", username, message);
    return 1; // Simulate success
}

// --- URL Decoder ---
// Decodes a URL-encoded string in place.
// Returns 0 on success, -1 on failure (e.g., invalid encoding).
int url_decode(char* str) {
    if (str == NULL) return -1;
    char* p = str;
    char* q = str;
    while (*p) {
        if (*p == '%') {
            if (p[1] && p[2] && isxdigit((unsigned char)p[1]) && isxdigit((unsigned char)p[2])) {
                char hex[3] = { p[1], p[2], '\0' };
                *q++ = (char)strtol(hex, NULL, 16);
                p += 3;
            } else {
                return -1; // Invalid hex escape
            }
        } else if (*p == '+') {
            *q++ = ' ';
            p++;
        } else {
            *q++ = *p++;
        }
    }
    *q = '\0';
    return 0;
}

// --- Core Logic for Testing ---
const char* handle_post_logic(const char* username, const char* message) {
    if (username == NULL || username[0] == '\0') {
        return "Username cannot be empty.";
    }
    if (message == NULL || message[0] == '\0') {
        return "Message cannot be empty.";
    }
    if (strlen(username) > MAX_USERNAME_LEN) {
        return "Username is too long.";
    }
    if (strlen(message) > MAX_MESSAGE_LEN) {
        return "Message is too long.";
    }
    
    // Basic username validation
    for (const char* p = username; *p; ++p) {
        if (!isalnum((unsigned char)*p) && *p != '_') {
            return "Username contains invalid characters.";
        }
    }

    if (insert_user_message_in_db(username, message)) {
        return "Success";
    } else {
        return "Failed to save message.";
    }
}

// --- `post()` function to handle form submission data ---
void post(int client_fd, const char* body) {
    char username[MAX_USERNAME_LEN + 1] = {0};
    char message[MAX_MESSAGE_LEN + 1] = {0};
    
    char* body_copy = strdup(body);
    if (body_copy == NULL) {
        perror("strdup failed");
        return; // Internal server error
    }

    // Using strtok_r for a slightly safer approach than strtok
    char* saveptr;
    char* param = strtok_r(body_copy, "&", &saveptr);
    while (param != NULL) {
        char* eq = strchr(param, '=');
        if (eq) {
            *eq = '\0';
            char* key = param;
            char* value = eq + 1;
            url_decode(value);

            if (strcmp(key, "username") == 0) {
                strncpy(username, value, MAX_USERNAME_LEN);
            } else if (strcmp(key, "message") == 0) {
                strncpy(message, value, MAX_MESSAGE_LEN);
            }
        }
        param = strtok_r(NULL, "&", &saveptr);
    }
    free(body_copy);
    
    const char* result = handle_post_logic(username, message);

    char response[512];
    if (strcmp(result, "Success") == 0) {
        snprintf(response, sizeof(response),
                 "HTTP/1.1 302 Found\r\n"
                 "Location: /\r\n"
                 "\r\n");
    } else {
         snprintf(response, sizeof(response),
                 "HTTP/1.1 400 Bad Request\r\n"
                 "Content-Type: text/html\r\n"
                 "\r\n"
                 "<html><body><h1>Error: %s</h1><a href=\"/\">Go back</a></body></html>", result);
    }
    send(client_fd, response, strlen(response), 0);
}

// --- Test Cases ---
void run_tests() {
    printf("--- Running Test Cases ---\n");
    // Test Case 1: Valid input
    printf("Test 1: %s\n", handle_post_logic("testuser", "Hello World!"));
    // Test Case 2: Empty username
    printf("Test 2: %s\n", handle_post_logic("", "A message from nowhere."));
    // Test Case 3: Empty message
    printf("Test 3: %s\n", handle_post_logic("anotheruser", ""));
    // Test Case 4: Username too long
    char long_username[MAX_USERNAME_LEN + 2];
    memset(long_username, 'a', sizeof(long_username) - 1);
    long_username[sizeof(long_username) - 1] = '\0';
    printf("Test 4: %s\n", handle_post_logic(long_username, "Short message."));
    // Test Case 5: Message too long
    char long_message[MAX_MESSAGE_LEN + 2];
    memset(long_message, 'b', sizeof(long_message) - 1);
    long_message[sizeof(long_message) - 1] = '\0';
    printf("Test 5: %s\n", handle_post_logic("shortuser", long_message));
    printf("--- Test Cases Finished ---\n\n");
}


int main() {
    run_tests();
    int server_fd, client_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);
    printf("Open http://localhost:%d in your browser.\n", PORT);

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        long valread = read(client_fd, buffer, BUFFER_SIZE - 1);
        if (valread <= 0) {
            close(client_fd);
            continue;
        }
        buffer[valread] = '\0'; // Null-terminate the buffer

        if (strncmp(buffer, "GET / ", 6) == 0) {
            const char *response =
                "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
                "<!DOCTYPE html><html><head><title>Message Board</title></head>"
                "<body><h1>Post a Message</h1>"
                "<form action=\"/post\" method=\"post\">"
                "<label for=\"username\">Username:</label><br>"
                "<input type=\"text\" id=\"username\" name=\"username\" required maxlength=\"32\"><br><br>"
                "<label for=\"message\">Message:</label><br>"
                "<textarea id=\"message\" name=\"message\" rows=\"4\" cols=\"50\" required maxlength=\"256\"></textarea><br><br>"
                "<input type=\"submit\" value=\"Submit\">"
                "</form></body></html>";
            send(client_fd, response, strlen(response), 0);
        } else if (strncmp(buffer, "POST /post ", 11) == 0) {
            char* body = strstr(buffer, "\r\n\r\n");
            if (body) {
                body += 4; // Move pointer past the newlines
                char* cl_header = strstr(buffer, "Content-Length: ");
                if (cl_header) {
                    long content_length = atol(cl_header + 16);
                    if (content_length > 0 && content_length < MAX_POST_BODY_SIZE) {
                         if (strlen(body) >= (size_t)content_length) {
                            body[content_length] = '\0'; // Truncate body to content length
                            post(client_fd, body);
                        }
                    }
                }
            }
        }
        
        shutdown(client_fd, SHUT_RDWR);
        close(client_fd);
    }
    close(server_fd);
    return 0;
}