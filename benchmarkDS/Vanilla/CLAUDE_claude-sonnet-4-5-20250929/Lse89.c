
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

typedef struct {
    char code[10];
    double price;
} Product;

Product products[] = {
    {"A001", 10.50},
    {"A002", 15.75},
    {"A003", 20.00},
    {"A004", 25.50},
    {"A005", 30.25}
};

double getPrice(const char* productCode) {
    for (int i = 0; i < 5; i++) {
        if (strcmp(products[i].code, productCode) == 0) {
            return products[i].price;
        }
    }
    return 0.0;
}

double calculateTotal(const char* productCode, int quantity) {
    return getPrice(productCode) * quantity;
}

void startServer() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(5000);
    
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 3);
    
    printf("Server started on port 5000\\n");
    
    while (1) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        char buffer[1024] = {0};
        read(new_socket, buffer, 1024);
        
        char response[] = "HTTP/1.1 200 OK\\r\\nContent-Type: text/plain\\r\\n\\r\\nWeb App Running";
        send(new_socket, response, strlen(response), 0);
        close(new_socket);
    }
}

int main() {
    // Test cases
    printf("Test Case 1: %.2f\\n", calculateTotal("A001", 2));
    printf("Test Case 2: %.2f\\n", calculateTotal("A002", 3));
    printf("Test Case 3: %.2f\\n", calculateTotal("A003", 1));
    printf("Test Case 4: %.2f\\n", calculateTotal("A004", 5));
    printf("Test Case 5: %.2f\\n", calculateTotal("A005", 4));
    
    // Uncomment to start server
    // startServer();
    
    return 0;
}
