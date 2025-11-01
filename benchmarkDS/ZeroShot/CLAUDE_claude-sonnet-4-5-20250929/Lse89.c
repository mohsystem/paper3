
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

typedef struct {
    char code[20];
    double price;
} Product;

Product products[] = {
    {"PROD001", 10.99},
    {"PROD002", 25.50},
    {"PROD003", 5.75},
    {"PROD004", 100.00},
    {"PROD005", 15.25}
};

int numProducts = 5;

double getPrice(const char* productCode) {
    for (int i = 0; i < numProducts; i++) {
        if (strcmp(products[i].code, productCode) == 0) {
            return products[i].price;
        }
    }
    return 0.0;
}

double calculatePrice(const char* productCode, int quantity) {
    if (quantity < 0) {
        return 0.0;
    }
    double price = getPrice(productCode);
    return price * quantity;
}

void parseQueryParam(const char* query, const char* param, char* value, size_t valueSize) {
    char searchStr[100];
    snprintf(searchStr, sizeof(searchStr), "%s=", param);
    
    const char* pos = strstr(query, searchStr);
    if (pos != NULL) {
        pos += strlen(searchStr);
        const char* end = strchr(pos, '&');
        size_t len = end ? (size_t)(end - pos) : strlen(pos);
        if (len < valueSize) {
            strncpy(value, pos, len);
            value[len] = '\\0';
        }
    } else {
        value[0] = '\\0';
    }
}

void handleRequest(int clientSocket, const char* request) {
    char response[4096];
    
    if (strstr(request, "GET /calculate") != NULL) {
        const char* queryStart = strchr(request, '?');
        if (queryStart != NULL) {
            queryStart++;
            char query[1024];
            const char* queryEnd = strchr(queryStart, ' ');
            size_t queryLen = queryEnd ? (size_t)(queryEnd - queryStart) : strlen(queryStart);
            strncpy(query, queryStart, queryLen);
            query[queryLen] = '\\0';
            
            char productCode[50] = {0};
            char quantityStr[20] = {0};
            
            parseQueryParam(query, "productCode", productCode, sizeof(productCode));
            parseQueryParam(query, "quantity", quantityStr, sizeof(quantityStr));
            
            if (strlen(productCode) > 0 && strlen(quantityStr) > 0) {
                int quantity = atoi(quantityStr);
                double totalPrice = calculatePrice(productCode, quantity);
                
                snprintf(response, sizeof(response),
                         "HTTP/1.1 200 OK\\r\\nContent-Type: application/json\\r\\n\\r\\n"
                         "{\\"productCode\\": \\"%s\\", \\"quantity\\": %d, \\"totalPrice\\": %.2f}",
                         productCode, quantity, totalPrice);
            } else {
                snprintf(response, sizeof(response),
                         "HTTP/1.1 400 Bad Request\\r\\nContent-Type: application/json\\r\\n\\r\\n"
                         "{\\"error\\": \\"Missing parameters\\"}");
            }
        } else {
            snprintf(response, sizeof(response),
                     "HTTP/1.1 400 Bad Request\\r\\nContent-Type: application/json\\r\\n\\r\\n"
                     "{\\"error\\": \\"No query parameters\\"}");
        }
    } else {
        snprintf(response, sizeof(response), "HTTP/1.1 404 Not Found\\r\\n\\r\\n");
    }
    
    send(clientSocket, response, strlen(response), 0);
}

int main() {
    printf("\\nTest Cases:\\n");
    printf("1. Product PROD001, Quantity 5: %.2f\\n", calculatePrice("PROD001", 5));
    printf("2. Product PROD002, Quantity 3: %.2f\\n", calculatePrice("PROD002", 3));
    printf("3. Product PROD003, Quantity 10: %.2f\\n", calculatePrice("PROD003", 10));
    printf("4. Product PROD999, Quantity 2: %.2f\\n", calculatePrice("PROD999", 2));
    printf("5. Product PROD004, Quantity 0: %.2f\\n", calculatePrice("PROD004", 0));
    
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("Socket creation failed");
        return 1;
    }
    
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(5000);
    
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind failed");
        close(serverSocket);
        return 1;
    }
    
    listen(serverSocket, 10);
    printf("\\nServer running on port 5000\\n");
    
    while (1) {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
        
        if (clientSocket >= 0) {
            char buffer[4096] = {0};
            read(clientSocket, buffer, sizeof(buffer));
            handleRequest(clientSocket, buffer);
            close(clientSocket);
        }
    }
    
    close(serverSocket);
    return 0;
}
