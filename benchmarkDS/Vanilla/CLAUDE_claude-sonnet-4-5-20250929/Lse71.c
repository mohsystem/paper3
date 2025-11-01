
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/stat.h>

#define PORT 8000
#define BUFFER_SIZE 8192
#define UPLOAD_DIR "uploads"

void create_upload_dir() {
    mkdir(UPLOAD_DIR, 0777);
}

void send_response(int client_socket, const char* response) {
    char http_response[BUFFER_SIZE];
    snprintf(http_response, sizeof(http_response),
             "HTTP/1.1 200 OK\\r\\n"
             "Content-Type: text/html\\r\\n"
             "Content-Length: %lu\\r\\n"
             "\\r\\n"
             "%s", strlen(response), response);
    
    send(client_socket, http_response, strlen(http_response), 0);
}

void handle_home(int client_socket) {
    const char* html = "<html><body>"
                      "<h2>File Upload Application</h2>"
                      "<form action='/upload' method='post' enctype='multipart/form-data'>"
                      "<input type='file' name='file' required><br><br>"
                      "<input type='submit' value='Upload File'>"
                      "</form></body></html>";
    send_response(client_socket, html);
}

void handle_upload(int client_socket, const char* request) {
    char filename[256] = "uploaded_file.txt";
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/%s", UPLOAD_DIR, filename);
    
    // Find content after headers
    const char* content_start = strstr(request, "\\r\\n\\r\\n");
    if (content_start != NULL) {
        content_start += 4;
        
        FILE* file = fopen(filepath, "wb");
        if (file != NULL) {
            fwrite(content_start, 1, strlen(content_start), file);
            fclose(file);
        }
    }
    
    char response[BUFFER_SIZE];
    snprintf(response, sizeof(response),
             "<html><body><h2>File uploaded successfully!</h2>"
             "<p>Filename: %s</p>"
             "<a href='/'>Upload another file</a></body></html>", filename);
    
    send_response(client_socket, response);
}

void handle_request(int client_socket) {
    char buffer[BUFFER_SIZE] = {0};
    int bytes_read = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    
    if (bytes_read > 0) {
        if (strncmp(buffer, "GET / ", 6) == 0) {
            handle_home(client_socket);
        } else if (strncmp(buffer, "POST /upload", 12) == 0) {
            handle_upload(client_socket, buffer);
        }
    }
}

void start_server() {
    create_upload_dir();
    
    int server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    
    printf("Server started on port %d\\n", PORT);
    printf("Open http://localhost:%d in your browser\\n", PORT);
    
    while (1) {
        client_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if (client_socket >= 0) {
            handle_request(client_socket);
            close(client_socket);
        }
    }
}

int main() {
    start_server();
    return 0;
}
