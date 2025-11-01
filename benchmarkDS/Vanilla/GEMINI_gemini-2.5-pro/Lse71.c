/*
 * WARNING: THIS IS A HIGHLY SIMPLIFIED AND INSECURE EXAMPLE.
 * This code is for demonstration purposes only and is not suitable for production.
 * It uses raw POSIX sockets and has minimal error handling, no security checks
 * (vulnerable to buffer overflows), and a very fragile multipart/form-data parser.
 * It is also not portable to Windows without using the Winsock API.
 * In a real-world scenario, use a dedicated library like libmicrohttpd or mongoose.
 *
 * COMPILE with: gcc Lse71.c -o Lse71
 *
 * RUN with: ./Lse71
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>

#define PORT 8080
#define BUFFER_SIZE 8192
#define UPLOAD_DIR "uploads"

// Helper function to find a substring in a buffer
char* find_str(const char* buffer, size_t buffer_len, const char* str) {
    return (char*)memmem(buffer, buffer_len, str, strlen(str));
}

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE] = {0};
    long valread = read(client_socket, buffer, BUFFER_SIZE - 1);
    
    if (valread < 0) {
        perror("read failed");
        close(client_socket);
        return;
    }
    
    // Check if it's a GET request for the root page
    if (strncmp(buffer, "GET / ", 6) == 0) {
        const char *html_form = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
                                "<!DOCTYPE html><html><head><title>File Upload</title></head>"
                                "<body><h1>Upload a File</h1>"
                                "<form action=\"/upload\" method=\"post\" enctype=\"multipart/form-data\">"
                                "Select file: <input type=\"file\" name=\"file\"><br>"
                                "<input type=\"submit\" value=\"Upload File\">"
                                "</form></body></html>";
        write(client_socket, html_form, strlen(html_form));
    }
    // Check if it's a POST request for upload
    else if (strncmp(buffer, "POST /upload", 12) == 0) {
        char* boundary_start = find_str(buffer, valread, "boundary=");
        if (!boundary_start) {
            const char *response = "HTTP/1.1 400 Bad Request\r\n\r\nBoundary not found.";
            write(client_socket, response, strlen(response));
            close(client_socket);
            return;
        }
        
        boundary_start += strlen("boundary=");
        char* boundary_end = find_str(boundary_start, valread - (boundary_start - buffer), "\r\n");
        if (!boundary_end) {
             const char *response = "HTTP/1.1 400 Bad Request\r\n\r\nMalformed boundary.";
            write(client_socket, response, strlen(response));
            close(client_socket);
            return;
        }

        char boundary[256];
        snprintf(boundary, sizeof(boundary), "--%.*s", (int)(boundary_end - boundary_start), boundary_start);

        char* filename_tag = find_str(buffer, valread, "filename=\"");
        if (!filename_tag) {
            const char *response = "HTTP/1.1 400 Bad Request\r\n\r\nFilename not found.";
            write(client_socket, response, strlen(response));
            close(client_socket);
            return;
        }
        filename_tag += strlen("filename=\"");
        char* filename_end = find_str(filename_tag, valread - (filename_tag-buffer), "\"");
        char filename[256];
        snprintf(filename, sizeof(filename), "%.*s", (int)(filename_end - filename_tag), filename_tag);

        char* file_content_start = find_str(buffer, valread, "\r\n\r\n");
        if(!file_content_start){
            const char *response = "HTTP/1.1 400 Bad Request\r\n\r\nCould not find file content.";
            write(client_socket, response, strlen(response));
            close(client_socket);
            return;
        }
        file_content_start += 4; // Move past the CRLFCRLF

        char* file_content_end = find_str(file_content_start, valread - (file_content_start - buffer), boundary);
        if(!file_content_end){
            // This happens if the file content spans multiple reads; this simple server doesn't handle it.
            // We'll just assume the boundary is at the end of the current buffer for simplicity.
            file_content_end = buffer + valread - strlen(boundary) - 4; // Heuristic
        }

        size_t content_length = file_content_end - file_content_start;
        
        char filepath[512];
        snprintf(filepath, sizeof(filepath), "%s/%s", UPLOAD_DIR, filename);
        
        FILE *fp = fopen(filepath, "wb");
        if (fp) {
            fwrite(file_content_start, 1, content_length, fp);
            fclose(fp);
            
            char success_msg[512];
            snprintf(success_msg, sizeof(success_msg), "HTTP/1.1 200 OK\r\n\r\nFile %s uploaded successfully.", filename);
            write(client_socket, success_msg, strlen(success_msg));
        } else {
            const char *response = "HTTP/1.1 500 Internal Server Error\r\n\r\nCould not save file.";
            write(client_socket, response, strlen(response));
        }
    } else {
        const char *response = "HTTP/1.1 404 Not Found\r\n\r\nNot Found.";
        write(client_socket, response, strlen(response));
    }

    close(client_socket);
}

void run_server() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    // Forcefully attaching socket to the port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    printf("Server started on port %d\n", PORT);
    printf("Visit http://localhost:%d to upload a file.\n", PORT);
    printf("Or use curl for testing from your terminal:\n");
    printf("-------------------- TEST CASES --------------------\n");
    printf("1. Create a test file: echo \"Test file 1 content\" > test1.txt\n");
    printf("2. Upload test1.txt: curl -F \"file=@test1.txt\" http://localhost:%d/upload\n", PORT);
    printf("3. Create another file: echo \"This is the second file.\" > test2.txt\n");
    printf("4. Upload test2.txt: curl -F \"file=@test2.txt\" http://localhost:%d/upload\n", PORT);
    printf("5. Try a GET request to upload (will show Not Found): curl http://localhost:%d/upload\n", PORT);
    printf("--------------------------------------------------\n");


    while(1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            continue; // Continue to the next iteration
        }
        handle_client(new_socket);
    }
}

int main() {
    // Create upload directory if it doesn't exist
    struct stat st = {0};
    if (stat(UPLOAD_DIR, &st) == -1) {
        mkdir(UPLOAD_DIR, 0700);
    }

    run_server();
    return 0;
}