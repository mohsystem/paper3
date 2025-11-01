
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    char product_code[20];
    double price;
} Product;

Product PRODUCT_PRICES[] = {
    {"PROD001", 10.50},
    {"PROD002", 25.99},
    {"PROD003", 5.75},
    {"PROD004", 100.00},
    {"PROD005", 15.25}
};

const int PRODUCT_COUNT = 5;

void trim(char* str) {
    char* end;
    while(*str == ' ' || *str == '\\t' || *str == '\\n' || *str == '\\r') str++;
    if(*str == 0) return;
    end = str + strlen(str) - 1;
    while(end > str && (*end == ' ' || *end == '\\t' || *end == '\\n' || *end == '\\r')) end--;
    *(end + 1) = 0;
}

double getProductPrice(const char* product_code) {
    for (int i = 0; i < PRODUCT_COUNT; i++) {
        if (strcmp(PRODUCT_PRICES[i].product_code, product_code) == 0) {
            return PRODUCT_PRICES[i].price;
        }
    }
    return -1.0;
}

void calculatePrice(const char* yaml_payload, char* result) {
    char payload_copy[1000];
    char product_code[50] = "";
    int quantity = 0;
    int found_product = 0, found_quantity = 0;
    
    if (yaml_payload == NULL || strlen(yaml_payload) == 0) {
        strcpy(result, "Error: Empty payload");
        return;
    }
    
    strncpy(payload_copy, yaml_payload, sizeof(payload_copy) - 1);
    payload_copy[sizeof(payload_copy) - 1] = '\\0';
    
    char* line = strtok(payload_copy, "\\n");
    while (line != NULL) {
        char* colon = strchr(line, ':');
        if (colon != NULL) {
            *colon = '\\0';
            char* key = line;
            char* value = colon + 1;
            
            trim(key);
            trim(value);
            
            if (strcmp(key, "product_code") == 0) {
                strncpy(product_code, value, sizeof(product_code) - 1);
                product_code[sizeof(product_code) - 1] = '\\0';
                found_product = 1;
            } else if (strcmp(key, "quantity") == 0) {
                quantity = atoi(value);
                found_quantity = 1;
            }
        }
        line = strtok(NULL, "\\n");
    }
    
    if (!found_product || !found_quantity) {
        strcpy(result, "Error: Missing product_code or quantity");
        return;
    }
    
    if (quantity <= 0) {
        strcpy(result, "Error: Quantity must be positive");
        return;
    }
    
    double price = getProductPrice(product_code);
    if (price < 0) {
        strcpy(result, "Error: Product not found");
        return;
    }
    
    double total_price = price * quantity;
    sprintf(result, "%.2f", total_price);
}

int main() {
    char result[200];
    
    // Test case 1: Valid product and quantity
    calculatePrice("product_code: PROD001\\nquantity: 3", result);
    printf("Test 1: %s\\n", result);
    
    // Test case 2: Different product
    calculatePrice("product_code: PROD002\\nquantity: 2", result);
    printf("Test 2: %s\\n", result);
    
    // Test case 3: Invalid product code
    calculatePrice("product_code: INVALID\\nquantity: 5", result);
    printf("Test 3: %s\\n", result);
    
    // Test case 4: Missing quantity
    calculatePrice("product_code: PROD003", result);
    printf("Test 4: %s\\n", result);
    
    // Test case 5: Zero quantity
    calculatePrice("product_code: PROD004\\nquantity: 0", result);
    printf("Test 5: %s\\n", result);
    
    return 0;
}
