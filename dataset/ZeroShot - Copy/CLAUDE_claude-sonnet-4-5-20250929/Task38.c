
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

void sanitizeInput(const char* input, char* output, int maxLen) {
    int j = 0;
    for (int i = 0; input[i] != '\\0' && j < maxLen - 10; i++) {
        switch (input[i]) {
            case '&':
                strcpy(output + j, "&amp;");
                j += 5;
                break;
            case '<':
                strcpy(output + j, "&lt;");
                j += 4;
                break;
            case '>':
                strcpy(output + j, "&gt;");
                j += 4;
                break;
            case '"':
                strcpy(output + j, "&quot;");
                j += 6;
                break;
            case '\\'':
                strcpy(output + j, "&#x27;");
                j += 6;
                break;
            case '/':
                strcpy(output + j, "&#x2F;");
                j += 6;
                break;
            default:
                output[j++] = input[i];
        }
    }
    output[j] = '\\0';
}

void urlDecode(char* dst, const char* src) {
    char a, b;
    while (*src) {
        if ((*src == '%') && ((a = src[1]) && (b = src[2])) && (isxdigit(a) && isxdigit(b))) {
            if (a >= 'a') a -= 'a'-'A';
            if (a >= 'A') a -= ('A' - 10);
            else a -= '0';
            if (b >= 'a') b -= 'a'-'A';
            if (b >= 'A') b -= ('A' - 10);
            else b -= '0';
            *dst++ = 16*a+b;
            src+=3;
        } else if (*src == '+') {
            *dst++ = ' ';
            src++;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\\0';
}

void parseFormData(const char* data, char* name, char* message) {
    char temp[2048];
    strcpy(temp, data);
    
    name[0] = '\\0';
    message[0] = '\\0';
    
    char* token = strtok(temp, "&");
    while (token != NULL) {
        char* eq = strchr(token, '=');
        if (eq != NULL) {
            *eq = '\\0';
            char* key = token;
            char* value = eq + 1;
            
            char decodedValue[1024];
            urlDecode(decodedValue, value);
            
            if (strcmp(key, "name") == 0) {
                strncpy(name, decodedValue, 255);
            } else if (strcmp(key, "message") == 0) {
                strncpy(message, decodedValue, 1023);
            }
        }
        token = strtok(NULL, "&");
    }
}

void sendFormHTML(int client_socket) {
    const char* response = 
        "HTTP/1.1 200 OK\\r\\nContent-Type: text/html; charset=utf-8\\r\\n\\r\\n"
        "<!DOCTYPE html><html><head><title>User Input Form</title>"
        "<style>body{font-family:Arial,sans-serif;max-width:600px;margin:50px auto;padding:20px;}"
        "input,textarea{width:100%;padding:10px;margin:10px 0;box-sizing:border-box;}"
        "button{background:#4CAF50;color:white;padding:10px 20px;border:none;cursor:pointer;}"
        "button:hover{background:#45a049;}</style></head><body>"
        "<h1>User Input Form</h1>"
        "<form method='POST' action='/submit'>"
        "<label>Name:</label><input type='text' name='name' required><br>"
        "<label>Message:</label><textarea name='message' rows='4' required></textarea><br>"
        "<button type='submit'>Submit</button></form></body></html>";
    
    send(client_socket, response, strlen(response), 0);
}

void sendResponseHTML(int client_socket, const char* name, const char* message) {
    char safeName[2048], safeMessage[4096];
    sanitizeInput(name, safeName, 2048);
    sanitizeInput(message, safeMessage, 4096);
    
    char response[8192];
    snprintf(response, sizeof(response),
        "HTTP/1.1 200 OK\\r\\nContent-Type: text/html; charset=utf-8\\r\\n\\r\\n"
        "<!DOCTYPE html><html><head><title>Your Response</title>"
        "<style>body{font-family:Arial,sans-serif;max-width:600px;margin:50px auto;padding:20px;}"
        ".result{background:#f0f0f0;padding:20px;border-radius:5px;margin:20px 0;}"
        "a{color:#4CAF50;text-decoration:none;}</style></head><body>"
        "<h1>Your Submission</h1><div class='result'>"
        "<p><strong>Name:</strong> %s</p>"
        "<p><strong>Message:</strong> %s</p></div>"
        "<a href='/'>← Back to Form</a></body></html>",
        safeName, safeMessage);
    
    send(client_socket, response, strlen(response), 0);
}

void handleRequest(int client_socket) {
    char buffer[4096] = {0};
    read(client_socket, buffer, 4096);
    
    if (strstr(buffer, "GET / ") == buffer || strstr(buffer, "GET /index") == buffer) {
        sendFormHTML(client_socket);
    } else if (strstr(buffer, "POST /submit") == buffer) {
        char* body = strstr(buffer, "\\r\\n\\r\\n");
        if (body != NULL) {
            body += 4;
            char name[256] = {0}, message[1024] = {0};
            parseFormData(body, name, message);
            sendResponseHTML(client_socket, name, message);
        }
    } else {
        const char* response = "HTTP/1.1 404 Not Found\\r\\n\\r\\n<html><body><h1>404 Not Found</h1></body></html>";
        send(client_socket, response, strlen(response), 0);
    }
    
    close(client_socket);
}

int main() {
    printf("Testing sanitizeInput function with 5 test cases:\\n\\n");
    
    const char* testCases[] = {
        "Hello World",
        "<script>alert('XSS')</script>",
        "Name with <b>HTML</b> tags",
        "Special chars: & < > \\" '",
        "Normal text without special characters"
    };
    
    for (int i = 0; i < 5; i++) {
        char sanitized[1024];
        sanitizeInput(testCases[i], sanitized, 1024);
        printf("Test Case %d:\\n", i + 1);
        printf("Input: %s\\n", testCases[i]);
        printf("Sanitized: %s\\n\\n", sanitized);
    }
    
    printf("\\nStarting web server on http://localhost:8000\\n");
    printf("Press Ctrl+C to stop the server\\n\\n");
    
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("Socket creation failed");
        return -1;
    }
    
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8000);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        return -1;
    }
    
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        return -1;
    }
    
    while (1) {
        int client_socket = accept(server_fd, NULL, NULL);
        if (client_socket < 0) continue;
        handleRequest(client_socket);
    }
    
    close(server_fd);
    return 0;
}
