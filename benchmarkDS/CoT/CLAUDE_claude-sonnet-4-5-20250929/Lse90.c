
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_QUANTITY 10000
#define MAX_YAML_SIZE 1024
#define MAX_LINE_LENGTH 256
#define MAX_KEY_LENGTH 64
#define MAX_VALUE_LENGTH 64

typedef struct {
    char key[MAX_KEY_LENGTH];
    char value[MAX_VALUE_LENGTH];
} KeyValue;

void trim(char* str) {
    if (str == NULL) return;
    
    char* start = str;
    while (*start && isspace((unsigned char)*start)) start++;
    
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

void toUpper(char* str) {
    if (str == NULL) return;
    for (int i = 0; str[i]; i++) {
        str[i] = toupper((unsigned char)str[i]);
    }
}

double getPriceForProductCode(const char* productCode) {
    if (productCode == NULL || strlen(productCode) == 0) {
        return 0.0;
    }
    
    char upperCode[MAX_VALUE_LENGTH];
    strncpy(upperCode, productCode, MAX_VALUE_LENGTH - 1);
    upperCode[MAX_VALUE_LENGTH - 1] = 0;
    trim(upperCode);
    toUpper(upperCode);
    
    if (strcmp(upperCode, "PROD001") == 0) return 10.99;
    if (strcmp(upperCode, "PROD002") == 0) return 25.50;
    if (strcmp(upperCode, "PROD003") == 0) return 15.75;
    if (strcmp(upperCode, "PROD004") == 0) return 8.99;
    if (strcmp(upperCode, "PROD005") == 0) return 99.99;
    
    return 0.0;
}

int parseYaml(const char* yaml, KeyValue* data, int maxItems) {
    if (yaml == NULL || data == NULL) return 0;
    
    char yamlCopy[MAX_YAML_SIZE];
    strncpy(yamlCopy, yaml, MAX_YAML_SIZE - 1);
    yamlCopy[MAX_YAML_SIZE - 1] = 0;
    
    int count = 0;
    char* line = strtok(yamlCopy, "\\n");
    
    while (line != NULL && count < maxItems) {
        char* colon = strchr(line, ':');
        if (colon != NULL) {
            *colon = 0;
            char* key = line;
            char* value = colon + 1;
            
            trim(key);
            trim(value);
            
            strncpy(data[count].key, key, MAX_KEY_LENGTH - 1);
            data[count].key[MAX_KEY_LENGTH - 1] = 0;
            
            strncpy(data[count].value, value, MAX_VALUE_LENGTH - 1);
            data[count].value[MAX_VALUE_LENGTH - 1] = 0;
            
            count++;
        }
        line = strtok(NULL, "\\n");
    }
    
    return count;
}

char* calculatePrice(const char* yamlPayload, char* result, size_t resultSize) {
    if (yamlPayload == NULL || result == NULL || resultSize == 0) {
        return NULL;
    }
    
    if (strlen(yamlPayload) == 0 || strlen(yamlPayload) > MAX_YAML_SIZE) {
        snprintf(result, resultSize, "error: Invalid payload size\\n");
        return result;
    }
    
    KeyValue data[10];
    int count = parseYaml(yamlPayload, data, 10);
    
    char productCode[MAX_VALUE_LENGTH] = "";
    int quantity = 0;
    int foundProduct = 0, foundQuantity = 0;
    
    for (int i = 0; i < count; i++) {
        if (strcmp(data[i].key, "product_code") == 0) {
            strncpy(productCode, data[i].value, MAX_VALUE_LENGTH - 1);
            productCode[MAX_VALUE_LENGTH - 1] = 0;
            foundProduct = 1;
        } else if (strcmp(data[i].key, "quantity") == 0) {
            char* endptr;
            long val = strtol(data[i].value, &endptr, 10);
            if (*endptr == 0 && val > 0 && val <= MAX_QUANTITY) {
                quantity = (int)val;
                foundQuantity = 1;
            }
        }
    }
    
    if (!foundProduct || !foundQuantity) {
        snprintf(result, resultSize, "error: Missing required fields\\n");
        return result;
    }
    
    if (quantity <= 0 || quantity > MAX_QUANTITY) {
        snprintf(result, resultSize, "error: Invalid quantity\\n");
        return result;
    }
    
    double unitPrice = getPriceForProductCode(productCode);
    if (unitPrice == 0.0) {
        snprintf(result, resultSize, "error: Product not found\\n");
        return result;
    }
    
    double totalPrice = unitPrice * quantity;
    
    snprintf(result, resultSize, 
             "product_code: %s\\nquantity: %d\\nunit_price: %.2f\\ntotal_price: %.2f\\n",
             productCode, quantity, unitPrice, totalPrice);
    
    return result;
}

int main() {
    char result[512];
    
    printf("Test Case 1: Valid product\\n");
    calculatePrice("product_code: PROD001\\nquantity: 5", result, sizeof(result));
    printf("%s\\n", result);
    
    printf("\\nTest Case 2: Different product\\n");
    calculatePrice("product_code: PROD003\\nquantity: 10", result, sizeof(result));
    printf("%s\\n", result);
    
    printf("\\nTest Case 3: Invalid product code\\n");
    calculatePrice("product_code: INVALID\\nquantity: 2", result, sizeof(result));
    printf("%s\\n", result);
    
    printf("\\nTest Case 4: Invalid quantity\\n");
    calculatePrice("product_code: PROD002\\nquantity: -5", result, sizeof(result));
    printf("%s\\n", result);
    
    printf("\\nTest Case 5: Missing fields\\n");
    calculatePrice("product_code: PROD001", result, sizeof(result));
    printf("%s\\n", result);
    
    return 0;
}
