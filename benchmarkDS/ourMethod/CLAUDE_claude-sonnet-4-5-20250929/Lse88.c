
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>

/* Maximum YAML payload size to prevent DoS (10KB) */
#define MAX_YAML_SIZE 10240

/* Maximum quantity to prevent overflow */
#define MAX_QUANTITY 10000
#define MIN_QUANTITY 1

/* Maximum product code length */
#define MAX_PRODUCT_CODE_LEN 50

/* Product structure */
typedef struct {
    char code[MAX_PRODUCT_CODE_LEN + 1];
    int price_cents;
} Product;

/* Hardcoded product catalog */
static const Product PRODUCTS[] = {
    {"PROD-001", 1050},  /* $10.50 */
    {"PROD-002", 2500},  /* $25.00 */
    {"PROD-003", 599},   /* $5.99 */
    {"PROD-004", 10000}, /* $100.00 */
    {"PROD-005", 1575}   /* $15.75 */
};
static const size_t NUM_PRODUCTS = sizeof(PRODUCTS) / sizeof(PRODUCTS[0]);

/* Secure string copy with null termination guarantee */
static void secure_strcpy(char* dest, size_t dest_size, const char* src) {
    if (dest == NULL || src == NULL || dest_size == 0) return;
    size_t i;
    for (i = 0; i < dest_size - 1 && src[i] != '\\0'; i++) {
        dest[i] = src[i];
    }
    dest[i] = '\\0';
}

/* Trim whitespace from string in-place */
static void trim(char* str) {
    if (str == NULL) return;
    
    /* Trim leading whitespace */
    char* start = str;
    while (*start && isspace((unsigned char)*start)) start++;
    
    /* Trim trailing whitespace */
    char* end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) end--;
    
    /* Move trimmed string to beginning and null terminate */
    size_t len = (end - start) + 1;
    if (start != str) {
        memmove(str, start, len);
    }
    str[len] = '\\0';
}

/* Validate product code format (alphanumeric and hyphens only) */
static int is_valid_product_code(const char* code) {
    if (code == NULL || code[0] == '\\0') return 0;
    
    size_t len = strlen(code);
    if (len > MAX_PRODUCT_CODE_LEN) return 0;
    
    for (size_t i = 0; i < len; i++) {
        char c = code[i];
        if (!isalnum((unsigned char)c) && c != '-') {
            return 0;
        }
    }
    return 1;
}

/* Find product price by code */
static int get_product_price(const char* code) {
    for (size_t i = 0; i < NUM_PRODUCTS; i++) {
        if (strcmp(PRODUCTS[i].code, code) == 0) {
            return PRODUCTS[i].price_cents;
        }
    }
    return -1;
}

/* Safe string to integer conversion with overflow checking */
static int safe_strtoint(const char* str, int* result) {
    if (str == NULL || result == NULL) return 0;
    
    errno = 0;
    char* endptr = NULL;
    long val = strtol(str, &endptr, 10);
    
    /* Check for conversion errors */
    if (errno == ERANGE || val < INT_MIN || val > INT_MAX) {
        return 0;
    }
    if (endptr == str || *endptr != '\\0') {
        return 0;
    }
    
    *result = (int)val;
    return 1;
}

/* Simple YAML parser for key-value pairs (secure implementation) */
static int parse_simple_yaml(const char* yaml, char* product_code, int* quantity) {
    if (yaml == NULL || product_code == NULL || quantity == NULL) return 0;
    
    int found_product = 0;
    int found_quantity = 0;
    
    /* Create a working copy to avoid modifying input */
    size_t yaml_len = strlen(yaml);
    char* yaml_copy = (char*)malloc(yaml_len + 1);
    if (yaml_copy == NULL) return 0;
    
    secure_strcpy(yaml_copy, yaml_len + 1, yaml);
    
    /* Parse line by line */
    char* line = strtok(yaml_copy, "\\n");
    while (line != NULL) {
        trim(line);
        
        if (strlen(line) > 0) {
            /* Find colon separator */
            char* colon = strchr(line, ':');
            if (colon != NULL) {
                *colon = '\\0';
                char* key = line;
                char* value = colon + 1;
                
                trim(key);
                trim(value);
                
                if (strcmp(key, "product_code") == 0) {
                    secure_strcpy(product_code, MAX_PRODUCT_CODE_LEN + 1, value);
                    found_product = 1;
                } else if (strcmp(key, "quantity") == 0) {
                    if (safe_strtoint(value, quantity)) {
                        found_quantity = 1;
                    } else {
                        free(yaml_copy);
                        return 0;
                    }
                }
            }
        }
        line = strtok(NULL, "\\n");
    }
    
    free(yaml_copy);
    return found_product && found_quantity;
}

/**
 * Calculates price based on YAML payload.
 * Security measures:
 * - Input size validation to prevent DoS
 * - Safe YAML parsing without entity expansion
 * - Bounds checking on all operations
 * - Type validation for all extracted values
 * - Range validation for quantity
 * - Whitelist validation for product codes
 * - Overflow prevention using checked arithmetic
 * - All buffers properly sized and null terminated
 * - Generic error messages to prevent information leakage
 */
char* calculate_price(const char* yaml_payload) {
    /* Allocate result buffer */
    char* result = (char*)malloc(256);
    if (result == NULL) {
        return NULL;
    }
    
    /* Initialize result buffer */
    result[0] = '\\0';
    
    /* Validate input is not NULL or empty */
    if (yaml_payload == NULL || yaml_payload[0] == '\\0') {
        secure_strcpy(result, 256, "Error: Invalid request");
        return result;
    }
    
    /* Validate payload size to prevent DoS attacks */
    size_t payload_len = strlen(yaml_payload);
    if (payload_len > MAX_YAML_SIZE) {
        secure_strcpy(result, 256, "Error: Request too large");
        return result;
    }
    
    char product_code[MAX_PRODUCT_CODE_LEN + 1] = {0};
    int quantity = 0;
    
    /* Parse YAML safely */
    if (!parse_simple_yaml(yaml_payload, product_code, &quantity)) {
        secure_strcpy(result, 256, "Error: Invalid format");
        return result;
    }
    
    /* Validate product code format */
    if (!is_valid_product_code(product_code)) {
        secure_strcpy(result, 256, "Error: Invalid product code format");
        return result;
    }
    
    /* Validate quantity range */
    if (quantity < MIN_QUANTITY || quantity > MAX_QUANTITY) {
        secure_strcpy(result, 256, "Error: Quantity out of range");
        return result;
    }
    
    /* Get product price */
    int unit_price_cents = get_product_price(product_code);
    if (unit_price_cents < 0) {
        secure_strcpy(result, 256, "Error: Product not found");
        return result;
    }
    
    /* Check for multiplication overflow before calculating */
    if (quantity > 0 && unit_price_cents > INT_MAX / quantity) {
        secure_strcpy(result, 256, "Error: Calculation overflow");
        return result;
    }
    
    /* Calculate total price in cents */
    int total_cents = unit_price_cents * quantity;
    
    /* Convert to dollars with 2 decimal places */
    int dollars = total_cents / 100;
    int cents = total_cents % 100;
    
    /* Format result with bounds checking */
    snprintf(result, 256, "%d.%02d", dollars, cents);
    
    return result;
}

int main(void) {
    printf("Test Case 1 - Valid request:\\n");
    const char* test1 = "product_code: PROD-001\\nquantity: 5";
    printf("Input: product_code: PROD-001 | quantity: 5\\n");
    char* output1 = calculate_price(test1);
    if (output1 != NULL) {
        printf("Output: %s\\n\\n", output1);
        free(output1);
    }
    
    printf("Test Case 2 - Different product:\\n");
    const char* test2 = "product_code: PROD-003\\nquantity: 10";
    printf("Input: product_code: PROD-003 | quantity: 10\\n");
    char* output2 = calculate_price(test2);
    if (output2 != NULL) {
        printf("Output: %s\\n\\n", output2);
        free(output2);
    }
    
    printf("Test Case 3 - Invalid product code:\\n");
    const char* test3 = "product_code: INVALID\\nquantity: 2";
    printf("Input: product_code: INVALID | quantity: 2\\n");
    char* output3 = calculate_price(test3);
    if (output3 != NULL) {
        printf("Output: %s\\n\\n", output3);
        free(output3);
    }
    
    printf("Test Case 4 - Quantity out of range:\\n");
    const char* test4 = "product_code: PROD-002\\nquantity: 20000";
    printf("Input: product_code: PROD-002 | quantity: 20000\\n");
    char* output4 = calculate_price(test4);
    if (output4 != NULL) {
        printf("Output: %s\\n\\n", output4);
        free(output4);
    }
    
    printf("Test Case 5 - Missing field:\\n");
    const char* test5 = "product_code: PROD-001";
    printf("Input: product_code: PROD-001\\n");
    char* output5 = calculate_price(test5);
    if (output5 != NULL) {
        printf("Output: %s\\n", output5);
        free(output5);
    }
    
    return 0;
}
