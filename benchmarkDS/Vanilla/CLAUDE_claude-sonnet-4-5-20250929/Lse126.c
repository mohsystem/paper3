
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

void http_hello_page(int client_socket, const char* request) {
    // Read the request body (username)
    char username[1024] = "";
    const char* body_start = strstr(request, "\\r\\n\\r\\n");
    if (body_start != NULL) {
        body_start += 4;
        strncpy(username, body_start, sizeof(username) - 1);
    }
    
    // Construct the response body (HTML page)
    char outputbuf[2048];
    snprintf(outputbuf, sizeof(outputbuf),
             "<!DOCTYPE html>\\n"
             "<html>\\n"
             "<head><title>Hello Page</title></head>\\n"
             "<body>\\n"
             "<h1>Hello, %s!</h1>\\n"
             "<p>Welcome to our server.</p>\\n"
             "</body>\\n"
             "</html>", username);
    
    // Construct HTTP response
    char response[4096];
    snprintf(response, sizeof(response),
             "HTTP/1.1 200 OK\\r\\n"
             "Content-Type: text/html\\r\\n"
             "Content-Length: %zu\\r\\n"
             "\\r\\n"
             "%s", strlen(outputbuf), outputbuf);
    
    // Write the response body to the response stream
    send(client_socket, response, strlen(response), 0);
    
    // Close the response stream
    close(client_socket);
}

int main() {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        fprintf(stderr, "Failed to create socket\\n");
        return 1;
    }
    
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);
    
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "Bind failed\\n");
        return 1;
    }
    
    if (listen(server_socket, 10) < 0) {
        fprintf(stderr, "Listen failed\\n");
        return 1;
    }
    
    printf("Server started on port 8080\\n");
    printf("Test the server with:\\n");
    printf("curl -X POST -d 'Alice' http://localhost:8080/hello\\n");
    printf("curl -X POST -d 'Bob' http://localhost:8080/hello\\n");
    printf("curl -X POST -d 'Charlie' http://localhost:8080/hello\\n");
    printf("curl -X POST -d 'David' http://localhost:8080/hello\\n");
    printf("curl -X POST -d 'Eve' http://localhost:8080/hello\\n");
    
    while (1) {
        int client_socket = accept(server_socket, NULL, NULL);
        if (client_socket < 0) {
            continue;
        }
        
        char buffer[4096] = {0};
        recv(client_socket, buffer, sizeof(buffer), 0);
        
        if (strstr(buffer, "POST /hello") != NULL) {
            http_hello_page(client_socket, buffer);
        } else {
            close(client_socket);
        }
    }
    
    close(server_socket);
    return 0;
}
