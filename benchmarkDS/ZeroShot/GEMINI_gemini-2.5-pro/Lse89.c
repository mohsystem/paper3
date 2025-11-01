/*
 * =============================== IMPORTANT NOTICE ===============================
 * This C code demonstrates a basic web server using raw sockets.
 * It is intended for educational purposes ONLY and is NOT secure or robust
 * enough for a production environment. It lacks many features of a proper
 * HTTP server, such as full request parsing, header handling, and resistance
 * to common web attacks (e.g., Slowloris).
 *
 * For any real-world application, please use a dedicated, well-tested
 * web server library like libmicrohttpd, Mongoose, or CivetWeb.
 *
 * Compilation command:
 * gcc -o Lse89_c Lse89.c -Wall -Wextra
 * ================================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define PORT 5000
#define BUFFER_SIZE 2048
#define MAX_CONNECTIONS 5

// In-memory product database
typedef struct {
    const char* code;
    double price;
} Product;

Product product_prices[] = {
    {"A123", 10.50},
    {"B456", 25.00},
    {"C789", 5.75},
    {"D101", 99.99},
    {NULL, 0.0} // Sentinel value
};

/**
 * Returns the price for a given product code.
 * @param product_code The code of the product.
 * @return The price, or -1.0 if not found.
 */
double get_price(const char* product_code) {
    if (product_code == NULL) return -1.0;
    for (int i = 0; product_prices[i].code != NULL; i++) {
        if (strcmp(product_prices[i].code, product_code) == 0) {
            return product_prices[i].price;
        }
    }
    return -1.0;
}

// Safely sends a response to the client
void send_response(int client_fd, const char* status, const char* content) {
    char response_buffer[1024];
    snprintf(response_buffer, sizeof(response_buffer),
             "HTTP/1.1 %s\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: %zu\r\n"
             "Connection: close\r\n\r\n"
             "%s",
             status, strlen(content), content);
    send(client_fd, response_buffer, strlen(response_buffer), 0);
}

// Handles an incoming client connection
void handle_connection(int client_fd) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);

    if (bytes_received <= 0) {
        close(client_fd);
        return;
    }
    buffer[bytes_received] = '\0';

    char* method = strtok(buffer, " ");
    char* path = strtok(NULL, " ");

    if (method == NULL || path == NULL || strcmp(method, "GET") != 0) {
        send_response(client_fd, "400 Bad Request", "{\"error\":\"Invalid request\"}");
        close(client_fd);
        return;
    }

    char* query_string = strchr(path, '?');
    if (query_string == NULL || strncmp(path, "/price?", 7) != 0) {
        send_response(client_fd, "404 Not Found", "{\"error\":\"Endpoint not found\"}");
        close(client_fd);
        return;
    }
    query_string++; // Move past '?'

    char* product_code = NULL;
    char* quantity_str = NULL;
    char* param = strtok(query_string, "&");
    while (param != NULL) {
        if (strncmp(param, "product_code=", 13) == 0) {
            product_code = param + 13;
        } else if (strncmp(param, "quantity=", 9) == 0) {
            quantity_str = param + 9;
        }
        param = strtok(NULL, "&");
    }

    if (product_code == NULL || quantity_str == NULL) {
        send_response(client_fd, "400 Bad Request", "{\"error\":\"Missing product_code or quantity parameter\"}");
        close(client_fd);
        return;
    }

    double price = get_price(product_code);
    if (price < 0) {
        send_response(client_fd, "404 Not Found", "{\"error\":\"Product code not found\"}");
        close(client_fd);
        return;
    }
    
    char* endptr;
    errno = 0;
    long quantity = strtol(quantity_str, &endptr, 10);

    if (errno != 0 || *endptr != '\0') {
        send_response(client_fd, "400 Bad Request", "{\"error\":\"Quantity must be a valid integer\"}");
        close(client_fd);
        return;
    }
    
    if (quantity <= 0) {
        send_response(client_fd, "400 Bad Request", "{\"error\":\"Quantity must be a positive integer\"}");
        close(client_fd);
        return;
    }

    double total_price = price * quantity;
    char content_buffer[256];
    snprintf(content_buffer, sizeof(content_buffer),
             "{\"product_code\":\"%s\", \"quantity\":%ld, \"total_price\":%.2f}",
             product_code, quantity, total_price);

    send_response(client_fd, "200 OK", content_buffer);
    close(client_fd);
}


int main(void) {
    int server_fd, client_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    if (listen(server_fd, MAX_CONNECTIONS) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("C server listening on port %d\n", PORT);
    printf("--- Test Cases (use curl or a browser) ---\n");
    printf("1. Valid: curl \"http://localhost:5000/price?product_code=A123&quantity=5\"\n");
    printf("2. Not Found: curl \"http://localhost:5000/price?product_code=X999&quantity=2\"\n");
    printf("3. Invalid Quantity (text): curl \"http://localhost:5000/price?product_code=B456&quantity=abc\"\n");
    printf("4. Invalid Quantity (negative): curl \"http://localhost:5000/price?product_code=C789&quantity=-3\"\n");
    printf("5. Missing Parameter: curl \"http://localhost:5000/price?product_code=A123\"\n");


    while (1) {
        if ((client_fd = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            continue; // Continue to next iteration
        }
        handle_connection(client_fd);
    }

    close(server_fd);
    return 0;
}