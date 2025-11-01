
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

double getPriceForProductCode(const char* productCode) {
    if (strcmp(productCode, "PROD001") == 0) return 10.99;
    if (strcmp(productCode, "PROD002") == 0) return 25.50;
    if (strcmp(productCode, "PROD003") == 0) return 5.75;
    if (strcmp(productCode, "PROD004") == 0) return 100.00;
    if (strcmp(productCode, "PROD005") == 0) return 15.25;
    return 0.0;
}

char* calculatePrice(const char* yamlPayload, char* result, size_t resultSize) {
    char productCode[50] = "";
    int quantity = 0;
    char tempPayload[1000];
    
    strncpy(tempPayload, yamlPayload, sizeof(tempPayload) - 1);
    tempPayload[sizeof(tempPayload) - 1] = '\\0';
    
    char* line = strtok(tempPayload, "\\n");
    while (line != NULL) {
        char* colon = strchr(line, ':');
        if (colon != NULL) {
            *colon = '\\0';
            char* key = line;
            char* value = colon + 1;
            
            while (*key == ' ' || *key == '\\t') key++;
            while (*value == ' ' || *value == '\\t') value++;
            
            if (strcmp(key, "product_code") == 0) {
                strncpy(productCode, value, sizeof(productCode) - 1);
                productCode[sizeof(productCode) - 1] = '\\0';
            } else if (strcmp(key, "quantity") == 0) {
                quantity = atoi(value);
            }
        }
        line = strtok(NULL, "\\n");
    }
    
    double price = getPriceForProductCode(productCode);
    double totalPrice = price * quantity;
    
    snprintf(result, resultSize, "product_code: %s\\nquantity: %d\\ntotal_price: %.2f\\n", 
             productCode, quantity, totalPrice);
    
    return result;
}

int main() {
    char result[500];
    
    // Test case 1
    printf("Test 1:\\n%s\\n", calculatePrice("product_code: PROD001\\nquantity: 2", result, sizeof(result)));
    
    // Test case 2
    printf("Test 2:\\n%s\\n", calculatePrice("product_code: PROD002\\nquantity: 5", result, sizeof(result)));
    
    // Test case 3
    printf("Test 3:\\n%s\\n", calculatePrice("product_code: PROD003\\nquantity: 10", result, sizeof(result)));
    
    // Test case 4
    printf("Test 4:\\n%s\\n", calculatePrice("product_code: PROD004\\nquantity: 1", result, sizeof(result)));
    
    // Test case 5
    printf("Test 5:\\n%s\\n", calculatePrice("product_code: PROD999\\nquantity: 3", result, sizeof(result)));
    
    return 0;
}
