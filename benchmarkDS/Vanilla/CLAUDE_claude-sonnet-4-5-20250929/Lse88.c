
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_PRODUCTS 10
#define MAX_LINE 256
#define MAX_KEY 50
#define MAX_VALUE 50

typedef struct {
    char code[20];
    double price;
} Product;

Product productCatalog[] = {
    {"PROD001", 10.99},
    {"PROD002", 25.50},
    {"PROD003", 5.75},
    {"PROD004", 100.00},
    {"PROD005", 15.25}
};

int catalogSize = 5;

void trim(char* str) {
    char* start = str;
    char* end;
    
    while (*start == ' ' || *start == '\\t') start++;
    
    if (*start == 0) {
        *str = 0;
        return;
    }
    
    end = start + strlen(start) - 1;
    while (end > start && (*end == ' ' || *end == '\\t' || *end == '\\n' || *end == '\\r')) end--;
    
    *(end + 1) = 0;
    memmove(str, start, strlen(start) + 1);
}

void parseYAMLLine(const char* line, char* key, char* value) {
    const char* colon = strchr(line, ':');
    if (colon != NULL) {
        int keyLen = colon - line;
        strncpy(key, line, keyLen);
        key[keyLen] = '\\0';
        strcpy(value, colon + 1);
        trim(key);
        trim(value);
    }
}

char* calculatePrice(const char* yamlPayload, char* result) {
    char payload[1024];
    strcpy(payload, yamlPayload);
    
    char productCode[50] = "";
    int quantity = 0;
    
    char* line = strtok(payload, "\\n");
    while (line != NULL) {
        char key[MAX_KEY];
        char value[MAX_VALUE];
        parseYAMLLine(line, key, value);
        
        if (strcmp(key, "product_code") == 0) {
            strcpy(productCode, value);
        } else if (strcmp(key, "quantity") == 0) {
            quantity = atoi(value);
        }
        
        line = strtok(NULL, "\\n");
    }
    
    double price = -1.0;
    for (int i = 0; i < catalogSize; i++) {
        if (strcmp(productCatalog[i].code, productCode) == 0) {
            price = productCatalog[i].price;
            break;
        }
    }
    
    if (price < 0) {
        strcpy(result, "Error: Product not found");
        return result;
    }
    
    double totalPrice = price * quantity;
    sprintf(result, "%.2f", totalPrice);
    return result;
}

int main() {
    char result[256];
    
    // Test case 1: Valid product with quantity 2
    char test1[] = "product_code: PROD001\\nquantity: 2";
    printf("Test 1: %s\\n", calculatePrice(test1, result));
    
    // Test case 2: Valid product with quantity 5
    char test2[] = "product_code: PROD002\\nquantity: 5";
    printf("Test 2: %s\\n", calculatePrice(test2, result));
    
    // Test case 3: Valid product with quantity 1
    char test3[] = "product_code: PROD003\\nquantity: 1";
    printf("Test 3: %s\\n", calculatePrice(test3, result));
    
    // Test case 4: Invalid product code
    char test4[] = "product_code: PROD999\\nquantity: 3";
    printf("Test 4: %s\\n", calculatePrice(test4, result));
    
    // Test case 5: Valid product with large quantity
    char test5[] = "product_code: PROD004\\nquantity: 10";
    printf("Test 5: %s\\n", calculatePrice(test5, result));
    
    return 0;
}
