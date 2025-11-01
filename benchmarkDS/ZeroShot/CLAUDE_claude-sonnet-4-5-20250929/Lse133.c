
// C doesn't have direct WSGI equivalent, using simple HTTP server approach
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

const char* getMessage() {
    return "Hello, World!";
}

void setup_server(int* server_fd, int port) {
    *server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (*server_fd < 0) {
        printf("Socket creation failed\\n");
        return;
    }
    
    int opt = 1;
    setsockopt(*server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    
    bind(*server_fd, (struct sockaddr*)&address, sizeof(address));
}

void run_server(int server_fd, int port) {
    listen(server_fd, 3);
    printf("Server started on port %d\\n", port);
}

int main() {
    // Test cases
    printf("Test Case 1: %s\\n", getMessage());
    printf("Test Case 2: %s\\n", getMessage());
    printf("Test Case 3: %s\\n", getMessage());
    printf("Test Case 4: %s\\n", getMessage());
    printf("Test Case 5: %s\\n", getMessage());
    
    // Uncomment to run the server
    // int server_fd;
    // setup_server(&server_fd, 8000);
    // run_server(server_fd, 8000);
    // close(server_fd);
    
    return 0;
}
