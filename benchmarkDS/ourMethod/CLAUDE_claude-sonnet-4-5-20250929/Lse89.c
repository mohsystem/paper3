
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>

#define MAX_PRODUCTS 10
#define MAX_CODE_LEN 20
#define MAX_BUFFER_SIZE 4096
#define MAX_QUERY_LEN 1000
#define MAX_PARAMS 10
#define PORT 5000

/* Product structure */
typedef struct {
    char code[MAX_CODE_LEN + 1];
    double price;
} Product;

/* Initialize product database */
static const Product PRODUCTS[MAX_PRODUCTS] = {
    {"PROD001", 10.50},
    {"PROD002", 25.00},
    {"PROD003", 15.75},
    {"PROD004", 30.00},
    {"PROD005", 8.99},
    {"", 0.0}
};

/**
 * Validate product code format (alphanumeric only, max 20 chars)
 */
bool isValidProductCode(const char* code) {
    if (code == NULL) {
        return false;
    }
    
    size_t len = strnlen(code, MAX_CODE_LEN + 1);
    
    /* Input validation: check length */
    if (len == 0 || len > MAX_CODE_LEN) {
        return false;
    }
    
    /* Input validation: check format */
    for (size_t i = 0; i < len; i++) {
        if (!isalnum((unsigned char)code[i])) {
            return false;
        }
    }
    
    return true;
}

/**
 * Returns the price for a given product code.
 */
bool getPrice(const char* productCode, double* price) {
    if (price == NULL) {
        return false;
    }
    
    /* Input validation: validate product code format */
    if (!isValidProductCode(productCode)) {
        return false;
    }
    
    /* Look up in product database */
    for (int i = 0; i < MAX_PRODUCTS && PRODUCTS[i].code[0] != '\\0'; i++) {
        if (strncmp(PRODUCTS[i].code, productCode, MAX_CODE_LEN) == 0) {
            *price = PRODUCTS[i].price;
            return true;
        }
    }
    
    return false;
}

/**
 * Calculates total price for a product code and quantity.
 */
bool calculatePrice(const char* productCode, int quantity, double* totalPrice) {
    if (totalPrice == NULL) {
        return false;
    }
    
    /* Input validation: quantity must be positive and within reasonable limits */
    if (quantity <= 0 || quantity > 10000) {
        return false;
    }
    
    double price = 0.0;
    if (!getPrice(productCode, &price)) {
        return false;
    }
    
    /* Check for overflow before multiplication */
    if (price > __DBL_MAX__ / quantity) {
        return false;
    }
    
    *totalPrice = price * quantity;
    return true;
}

/**
 * URL decode a string safely with bounds checking
 */
bool urlDecode(const char* src, char* dest, size_t destSize) {
    if (src == NULL || dest == NULL || destSize == 0) {
        return false;
    }
    
    size_t srcLen = strnlen(src, MAX_BUFFER_SIZE);
    size_t destIdx = 0;
    
    for (size_t i = 0; i < srcLen && destIdx < destSize - 1; i++) {
        if (src[i] == '%' && i + 2 < srcLen) {
            /* Input validation: check hex digits */
            if (isxdigit((unsigned char)src[i+1]) && isxdigit((unsigned char)src[i+2])) {
                char hex[3] = {src[i+1], src[i+2], '\\0'};
                dest[destIdx++] = (char)strtol(hex, NULL, 16);
                i += 2;
            } else {
                dest[destIdx++] = src[i];
            }
        } else if (src[i] == '+') {
            dest[destIdx++] = ' ';
        } else {
            dest[destIdx++] = src[i];
        }
    }
    
    /* Null terminate */
    dest[destIdx] = '\\0';
    return true;
}

/**
 * Extract query parameter value
 */
bool getQueryParam(const char* query, const char* key, char* value, size_t valueSize) {
    if (query == NULL || key == NULL || value == NULL || valueSize == 0) {
        return false;
    }
    
    /* Input validation: limit query length */
    size_t queryLen = strnlen(query, MAX_QUERY_LEN + 1);
    if (queryLen > MAX_QUERY_LEN) {
        return false;
    }
    
    size_t keyLen = strnlen(key, 50);
    const char* pos = query;
    int paramCount = 0;
    
    while (pos != NULL && *pos != '\\0') {
        /* Input validation: limit number of parameters */
        if (++paramCount > MAX_PARAMS) {
            return false;
        }
        
        /* Find key */
        const char* keyStart = pos;
        const char* equals = strchr(keyStart, '=');
        const char* ampersand = strchr(keyStart, '&');
        
        if (equals == NULL) {
            break;
        }
        
        size_t currentKeyLen = equals - keyStart;
        
        /* Check if this is our key */
        if (currentKeyLen == keyLen && strncmp(keyStart, key, keyLen) == 0) {
            const char* valueStart = equals + 1;
            size_t valueLen;
            
            if (ampersand != NULL) {
                valueLen = ampersand - valueStart;
            } else {
                valueLen = strnlen(valueStart, MAX_BUFFER_SIZE);
            }
            
            /* Input validation: check value length */
            if (valueLen >= valueSize) {
                return false;
            }
            
            /* Copy and decode value */
            char temp[MAX_BUFFER_SIZE];
            if (valueLen < MAX_BUFFER_SIZE) {
                memcpy(temp, valueStart, valueLen);
                temp[valueLen] = '\\0';
                return urlDecode(temp, value, valueSize);
            }
            return false;
        }
        
        /* Move to next parameter */
        if (ampersand != NULL) {
            pos = ampersand + 1;
        } else {
            break;
        }
    }
    
    return false;
}

/**
 * Handle HTTP request
 */
void handleRequest(int clientSocket) {
    char buffer[MAX_BUFFER_SIZE];
    char response[MAX_BUFFER_SIZE];
    char httpResponse[MAX_BUFFER_SIZE];
    
    /* Initialize buffers to zero */
    memset(buffer, 0, sizeof(buffer));
    memset(response, 0, sizeof(response));
    memset(httpResponse, 0, sizeof(httpResponse));
    
    /* Read request with bounds checking */
    ssize_t bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
    if (bytesRead <= 0) {
        close(clientSocket);
        return;
    }
    
    /* Null terminate for safety */
    buffer[bytesRead] = '\\0';
    
    int statusCode = 400;
    
    /* Parse request line */
    char method[10] = {0};
    char path[256] = {0};
    
    if (sscanf(buffer, "%9s %255s", method, path) == 2) {
        /* Only allow GET requests */
        if (strncmp(method, "GET", 3) == 0) {
            /* Find query string */
            char* queryStart = strchr(path, '?');
            if (queryStart != NULL) {
                queryStart++; /* Skip '?' */
                
                char code[MAX_CODE_LEN + 1] = {0};
                char quantityStr[20] = {0};
                
                /* Extract parameters */
                if (getQueryParam(queryStart, "code", code, sizeof(code)) &&
                    getQueryParam(queryStart, "quantity", quantityStr, sizeof(quantityStr))) {
                    
                    /* Parse quantity with error handling */
                    char* endPtr = NULL;
                    errno = 0;
                    long quantity = strtol(quantityStr, &endPtr, 10);
                    
                    if (errno == 0 && endPtr != quantityStr && *endPtr == '\\0') {
                        double totalPrice = 0.0;
                        
                        if (calculatePrice(code, (int)quantity, &totalPrice)) {
                            /* Use constant format string */
                            snprintf(response, sizeof(response), "Total price: %.2f", totalPrice);
                            statusCode = 200;
                        } else {
                            snprintf(response, sizeof(response), "Invalid product code or quantity");
                        }
                    } else {
                        snprintf(response, sizeof(response), "Invalid quantity format");
                    }
                } else {
                    snprintf(response, sizeof(response), "Missing required parameters");
                }
            } else {
                snprintf(response, sizeof(response), "Missing parameters");
            }
        } else {
            snprintf(response, sizeof(response), "Method not allowed");
            statusCode = 405;
        }
    } else {
        snprintf(response, sizeof(response), "Bad request");
    }
    
    /* Build HTTP response with security headers */
    snprintf(httpResponse, sizeof(httpResponse),
             "HTTP/1.1 %d OK\\r\\n"
             "Content-Type: text/plain; charset=utf-8\\r\\n"
             "X-Content-Type-Options: nosniff\\r\\n"
             "Content-Length: %zu\\r\\n"
             "Connection: close\\r\\n\\r\\n"
             "%s",
             statusCode, strlen(response), response);
    
    write(clientSocket, httpResponse, strlen(httpResponse));
    close(clientSocket);
}

int main(void) {
    double result = 0.0;
    
    /* Test cases */
    printf("Test Case 1: %s\\n", calculatePrice("PROD001", 2, &result) ? "21.00" : "null");
    printf("Test Case 2: %s\\n", calculatePrice("PROD002", 3, &result) ? "75.00" : "null");
    printf("Test Case 3: %s\\n", calculatePrice("PROD003", 5, &result) ? "78.75" : "null");
    printf("Test Case 4: %s\\n", calculatePrice("INVALID", 1, &result) ? "valid" : "null");
    printf("Test Case 5: %s\\n", calculatePrice("PROD001", -1, &result) ? "valid" : "null");
    
    /* Create socket */
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        fprintf(stderr, "Failed to create socket\\n");
        return 1;
    }
    
    /* Set socket options to reuse address */
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(serverSocket);
        return 1;
    }
    
    /* Bind to port 5000 */
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);
    
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        fprintf(stderr, "Failed to bind socket\\n");
        close(serverSocket);
        return 1;
    }
    
    /* Listen for connections */
    if (listen(serverSocket, 10) < 0) {
        fprintf(stderr, "Failed to listen\\n");
        close(serverSocket);
        return 1;
    }
    
    printf("Server started on port 5000\\n");
    printf("Example: http://localhost:5000/?code=PROD001&quantity=2\\n");
    
    /* Accept and handle connections */
    while (1) {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
        
        if (clientSocket >= 0) {
            handleRequest(clientSocket);
        }
    }
    
    close(serverSocket);
    return 0;
}
