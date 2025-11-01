/*
 * This is a minimal, non-robust HTTP server written in C using standard sockets.
 * It's for demonstration purposes and lacks proper error handling, security, and full HTTP compliance.
 * To compile: gcc -o server Lse54.c
 * To run: ./server
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8082
#define BUFFER_SIZE 2048

// Function to handle GET /login requests
void handle_login_get(int client_socket) {
    const char* form_html = "<html><body>"
                            "<h2>Login</h2>"
                            "<form method='POST' action='/do_login'>"
                            "Username: <input type='text' name='username'><br>"
                            "Password: <input type='password' name='password'><br>"
                            "<input type='submit' value='Login'>"
                            "</form>"
                            "</body></html>";
    char response[BUFFER_SIZE];
    sprintf(response, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %zu\r\n\r\n%s", strlen(form_html), form_html);
    send(client_socket, response, strlen(response), 0);
}

// Function to handle POST /do_login requests
void handle_do_login_post(int client_socket, const char* request_body) {
    if (strstr(request_body, "username=admin") && strstr(request_body, "password=admin")) {
        // Correct credentials, send redirect
        const char* redirect_header = "HTTP/1.1 302 Found\r\nLocation: /admin_page\r\n\r\n";
        send(client_socket, redirect_header, strlen(redirect_header), 0);
    } else {
        // Incorrect credentials
        const char* failed_msg = "Login Failed: Invalid credentials.";
        char response[BUFFER_SIZE];
        sprintf(response, "HTTP/1.1 401 Unauthorized\r\nContent-Type: text/plain\r\nContent-Length: %zu\r\n\r\n%s", strlen(failed_msg), failed_msg);
        send(client_socket, response, strlen(response), 0);
    }
}

// Function to handle GET /admin_page requests
void handle_admin_page_get(int client_socket) {
    const char* admin_html = "<html><body><h1>Welcome, Admin!</h1></body></html>";
    char response[BUFFER_SIZE];
    sprintf(response, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %zu\r\n\r\n%s", strlen(admin_html), admin_html);
    send(client_socket, response, strlen(response), 0);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Binding the socket to the network address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("C server started on port %d\n", PORT);
    printf("--- 5 Test Cases ---\n");
    printf("1. View login form: Open http://localhost:%d/login in a browser or run: curl http://localhost:%d/login\n", PORT, PORT);
    printf("2. Successful login: curl -X POST -d \"username=admin&password=admin\" -L http://localhost:%d/do_login\n", PORT);
    printf("3. Failed login (wrong user): curl -X POST -d \"username=user&password=admin\" http://localhost:%d/do_login\n", PORT);
    printf("4. Failed login (wrong pass): curl -X POST -d \"username=admin&password=wrong\" http://localhost:%d/do_login\n", PORT);
    printf("5. View admin page directly: curl http://localhost:%d/admin_page\n", PORT);


    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            continue; // Continue to next iteration
        }

        read(new_socket, buffer, BUFFER_SIZE);
        
        // Very basic request routing
        if (strncmp(buffer, "GET /login", 10) == 0) {
            handle_login_get(new_socket);
        } else if (strncmp(buffer, "POST /do_login", 14) == 0) {
            char* body = strstr(buffer, "\r\n\r\n");
            if (body) {
                body += 4; // Move pointer past the CRLF CRLF
                handle_do_login_post(new_socket, body);
            }
        } else if (strncmp(buffer, "GET /admin_page", 15) == 0) {
            handle_admin_page_get(new_socket);
        } else {
            const char* not_found_msg = "404 Not Found";
            char response[1024];
            sprintf(response, "HTTP/1.1 404 Not Found\r\nContent-Length: %zu\r\n\r\n%s", strlen(not_found_msg), not_found_msg);
            send(new_socket, response, strlen(response), 0);
        }

        close(new_socket);
    }

    return 0;
}