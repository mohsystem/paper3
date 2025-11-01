
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define MAX_PRODUCTS 10
#define MAX_CODE_LEN 20
#define PORT 5000

typedef struct {
    char code[MAX_CODE_LEN];
    double price;
} Product;

Product products[MAX_PRODUCTS];
int product_count = 0;

void initProducts() {
    strcpy(products[0].code, "P001");
    products[0].price = 10.99;
    strcpy(products[1].code, "P002");
    products[1].price = 25.50;
    strcpy(products[2].code, "P003");
    products[2].price = 15.75;
    strcpy(products[3].code, "P004");
    products[3].price = 30.00;
    strcpy(products[4].code, "P005");
    products[4].price = 5.99;
    product_count = 5;
}

void toUpperCase(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = toupper((unsigned char)str[i]);
    }
}

int isValidProductCode(const char* code) {
    if (!code || strlen(code) == 0) return 0;
    
    for (int i = 0; code[i]; i++) {
        if (!isalnum((unsigned char)code[i])) {
            return 0;
        }
    }
    return 1;
}

double getProductPrice(const char* productCode) {
    if (!isValidProductCode(productCode)) {
        return -1.0;
    }
    
    char upperCode[MAX_CODE_LEN];
    strncpy(upperCode, productCode, MAX_CODE_LEN - 1);
    upperCode[MAX_CODE_LEN - 1] = '\\0';
    toUpperCase(upperCode);
    
    for (int i = 0; i < product_count; i++) {
        if (strcmp(products[i].code, upperCode) == 0) {
            return products[i].price;
        }
    }
    return -1.0;
}

void calculatePrice(const char* productCode, const char* quantityStr, char* result, size_t resultSize) {
    if (!productCode || !quantityStr || strlen(productCode) == 0 || strlen(quantityStr) == 0) {
        snprintf(result, resultSize, "Error: Missing parameters");
        return;
    }
    
    // Validate quantity string contains only digits
    for (int i = 0; quantityStr[i]; i++) {
        if (!isdigit((unsigned char)quantityStr[i])) {
            snprintf(result, resultSize, "Error: Invalid quantity format");
            return;
        }
    }
    
    long quantity = atol(quantityStr);
    if (quantity <= 0 || quantity > 10000) {
        snprintf(result, resultSize, "Error: Invalid quantity. Must be between 1 and 10000");
        return;
    }
    
    double price = getProductPrice(productCode);
    if (price < 0) {
        snprintf(result, resultSize, "Error: Product not found");
        return;
    }
    
    double totalPrice = price * quantity;
    char upperCode[MAX_CODE_LEN];
    strncpy(upperCode, productCode, MAX_CODE_LEN - 1);
    upperCode[MAX_CODE_LEN - 1] = '\\0';
    toUpperCase(upperCode);
    
    snprintf(result, resultSize, "Product: %s, Quantity: %ld, Total Price: $%.2f", 
             upperCode, quantity, totalPrice);
}

int main() {
    initProducts();
    
    // Test cases
    char result[256];
    printf("\\nTest Cases:\\n");
    
    calculatePrice("P001", "5", result, sizeof(result));
    printf("1. %s\\n", result);
    
    calculatePrice("P003", "10", result, sizeof(result));
    printf("2. %s\\n", result);
    
    calculatePrice("P999", "5", result, sizeof(result));
    printf("3. %s\\n", result);
    
    calculatePrice("P002", "0", result, sizeof(result));
    printf("4. %s\\n", result);
    
    calculatePrice("P001", "abc", result, sizeof(result));
    printf("5. %s\\n", result);
    
    printf("\\nNote: Full web server functionality requires external HTTP library\\n");
    printf("Server would run on port %d\\n", PORT);
    
    return 0;
}
