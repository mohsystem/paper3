
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_PRODUCTS 10
#define MAX_LINE 256
#define MAX_KEY 50
#define MAX_VALUE 50

typedef struct {
    char code[20];
    double price;
} Product;

Product products[MAX_PRODUCTS];
int productCount = 0;

void initializePrices() {
    strcpy(products[0].code, "PROD001");
    products[0].price = 10.50;
    strcpy(products[1].code, "PROD002");
    products[1].price = 25.00;
    strcpy(products[2].code, "PROD003");
    products[2].price = 15.75;
    strcpy(products[3].code, "PROD004");
    products[3].price = 30.00;
    strcpy(products[4].code, "PROD005");
    products[4].price = 5.99;
    productCount = 5;
}

void trimString(char* str) {
    char* start = str;
    char* end;
    
    while(isspace((unsigned char)*start)) start++;
    
    if(*start == 0) {
        str[0] = '\\0';
        return;
    }
    
    end = start + strlen(start) - 1;
    while(end > start && isspace((unsigned char)*end)) end--;
    
    *(end + 1) = '\\0';
    memmove(str, start, end - start + 2);
}

double getProductPrice(const char* productCode) {
    for(int i = 0; i < productCount; i++) {
        if(strcmp(products[i].code, productCode) == 0) {
            return products[i].price;
        }
    }
    return -1.0;
}

int parseSimpleYAML(const char* yaml, char* productCode, int* quantity) {
    char buffer[1024];
    strncpy(buffer, yaml, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\\0';
    
    int hasProductCode = 0;
    int hasQuantity = 0;
    
    char* line = strtok(buffer, "\\n");
    while(line != NULL) {
        char* colon = strchr(line, ':');
        if(colon != NULL) {
            *colon = '\\0';
            char* key = line;
            char* value = colon + 1;
            
            trimString(key);
            trimString(value);
            
            if(strcmp(key, "product_code") == 0) {
                strncpy(productCode, value, 19);
                productCode[19] = '\\0';
                hasProductCode = 1;
            } else if(strcmp(key, "quantity") == 0) {
                *quantity = atoi(value);
                hasQuantity = 1;
            }
        }
        line = strtok(NULL, "\\n");
    }
    
    return hasProductCode && hasQuantity;
}

void calculatePrice(const char* yamlPayload, char* result, size_t resultSize) {
    char productCode[20] = {0};
    int quantity = 0;
    
    if(!parseSimpleYAML(yamlPayload, productCode, &quantity)) {
        snprintf(result, resultSize, "Error: Invalid YAML format or missing fields");
        return;
    }
    
    double price = getProductPrice(productCode);
    if(price < 0) {
        snprintf(result, resultSize, "Error: Invalid product code");
        return;
    }
    
    if(quantity <= 0 || quantity > 10000) {
        snprintf(result, resultSize, "Error: Quantity must be between 1 and 10000");
        return;
    }
    
    double totalPrice = price * quantity;
    snprintf(result, resultSize, "%.2f", totalPrice);
}

int main() {
    initializePrices();
    char result[256];
    
    // Test case 1: Valid input
    calculatePrice("product_code: PROD001\\nquantity: 5", result, sizeof(result));
    printf("Test 1: %s\\n", result);
    
    // Test case 2: Different product
    calculatePrice("product_code: PROD003\\nquantity: 10", result, sizeof(result));
    printf("Test 2: %s\\n", result);
    
    // Test case 3: Invalid product code
    calculatePrice("product_code: INVALID\\nquantity: 3", result, sizeof(result));
    printf("Test 3: %s\\n", result);
    
    // Test case 4: Invalid quantity
    calculatePrice("product_code: PROD002\\nquantity: -5", result, sizeof(result));
    printf("Test 4: %s\\n", result);
    
    // Test case 5: Missing fields
    calculatePrice("product_code: PROD001", result, sizeof(result));
    printf("Test 5: %s\\n", result);
    
    return 0;
}
