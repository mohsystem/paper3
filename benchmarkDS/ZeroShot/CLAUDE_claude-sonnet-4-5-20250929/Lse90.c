
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_PRODUCTS 5
#define MAX_CODE_LEN 20
#define MAX_PAYLOAD_LEN 1024
#define MAX_RESPONSE_LEN 2048

typedef struct {
    char code[MAX_CODE_LEN];
    double price;
} Product;

static Product products[MAX_PRODUCTS] = {
    {"PROD001", 10.99},
    {"PROD002", 25.50},
    {"PROD003", 15.75},
    {"PROD004", 8.99},
    {"PROD005", 100.00}
};

void toUpper(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = toupper((unsigned char)str[i]);
    }
}

void trim(char* str) {
    char* start = str;
    while (isspace((unsigned char)*start)) start++;
    
    if (*start == 0) {
        *str = 0;
        return;
    }
    
    char* end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) end--;
    
    size_t len = end - start + 1;
    memmove(str, start, len);
    str[len] = 0;
}

double getPriceForProductCode(const char* productCode) {
    if (productCode == NULL || strlen(productCode) == 0) {
        return -1.0;
    }
    
    char code[MAX_CODE_LEN];
    strncpy(code, productCode, MAX_CODE_LEN - 1);
    code[MAX_CODE_LEN - 1] = '\\0';
    trim(code);
    toUpper(code);
    
    for (int i = 0; i < MAX_PRODUCTS; i++) {
        if (strcmp(code, products[i].code) == 0) {
            return products[i].price;
        }
    }
    return -1.0;
}

void createErrorResponse(char* output, const char* message) {
    snprintf(output, MAX_RESPONSE_LEN, "error: %s\\n", message);
}

int parseYamlField(const char* yaml, const char* field, char* value, size_t valueSize) {
    char searchStr[50];
    snprintf(searchStr, sizeof(searchStr), "%s:", field);
    
    const char* pos = strstr(yaml, searchStr);
    if (pos == NULL) {
        return 0;
    }
    
    pos += strlen(searchStr);
    while (*pos == ' ' || *pos == '\\t') pos++;
    
    size_t i = 0;
    while (*pos && *pos != '\\n' && *pos != '\\r' && i < valueSize - 1) {
        value[i++] = *pos++;
    }
    value[i] = '\\0';
    trim(value);
    return 1;
}

void calculatePrice(const char* yamlPayload, char* output) {
    if (yamlPayload == NULL || strlen(yamlPayload) == 0) {
        createErrorResponse(output, "Invalid payload");
        return;
    }
    
    char productCode[MAX_CODE_LEN];
    char quantityStr[20];
    
    if (!parseYamlField(yamlPayload, "product_code", productCode, sizeof(productCode))) {
        createErrorResponse(output, "Missing product_code field");
        return;
    }
    
    if (!parseYamlField(yamlPayload, "quantity", quantityStr, sizeof(quantityStr))) {
        createErrorResponse(output, "Missing quantity field");
        return;
    }
    
    int quantity = atoi(quantityStr);
    if (quantity <= 0) {
        createErrorResponse(output, "Quantity must be positive");
        return;
    }
    
    double price = getPriceForProductCode(productCode);
    if (price < 0) {
        createErrorResponse(output, "Product not found");
        return;
    }
    
    double totalPrice = price * quantity;
    
    snprintf(output, MAX_RESPONSE_LEN,
             "product_code: %s\\nquantity: %d\\nunit_price: %.2f\\ntotal_price: %.2f\\n",
             productCode, quantity, price, totalPrice);
}

int main() {
    char output[MAX_RESPONSE_LEN];
    
    printf("Test Case 1: Valid request\\n");
    calculatePrice("product_code: PROD001\\nquantity: 5", output);
    printf("%s\\n", output);
    
    printf("Test Case 2: Different product\\n");
    calculatePrice("product_code: PROD003\\nquantity: 3", output);
    printf("%s\\n", output);
    
    printf("Test Case 3: Invalid product code\\n");
    calculatePrice("product_code: INVALID\\nquantity: 2", output);
    printf("%s\\n", output);
    
    printf("Test Case 4: Missing fields\\n");
    calculatePrice("product_code: PROD001", output);
    printf("%s\\n", output);
    
    printf("Test Case 5: Negative quantity\\n");
    calculatePrice("product_code: PROD002\\nquantity: -5", output);
    printf("%s\\n", output);
    
    return 0;
}
