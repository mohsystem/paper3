
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

/* Maximum payload size to prevent memory exhaustion (1KB) */
#define MAX_PAYLOAD_SIZE 1024

/* Maximum quantity to prevent integer overflow */
#define MAX_QUANTITY 1000000

/* Maximum product code length */
#define MAX_PRODUCT_CODE_LEN 20

/* Maximum response size */
#define MAX_RESPONSE_SIZE 512

/* Price structure */
typedef struct {
    char code[MAX_PRODUCT_CODE_LEN + 1];
    double price;
} Product;

/* Hardcoded price database (in production, use a secure database) */
static const Product PRICE_DATABASE[] = {
    {"PROD001", 10.50},
    {"PROD002", 25.99},
    {"PROD003", 5.75},
    {"PROD004", 100.00},
    {"PROD005", 15.25}
};
static const size_t DB_SIZE = sizeof(PRICE_DATABASE) / sizeof(Product);

/* Trim whitespace from string in place */
static void trim(char* str) {
    if (str == NULL) return;
    
    char* start = str;
    while (*start && isspace((unsigned char)*start)) start++;
    
    if (*start == '\\0') {
        str[0] = '\\0';
        return;
    }
    
    char* end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) end--;
    
    size_t len = end - start + 1;
    memmove(str, start, len);
    str[len] = '\\0';
}

/* Validate product code format (alphanumeric only) */
static int is_valid_product_code(const char* code) {
    if (code == NULL || code[0] == '\\0') return 0;
    
    size_t len = strlen(code);
    if (len > MAX_PRODUCT_CODE_LEN) return 0;
    
    for (size_t i = 0; i < len; i++) {
        if (!isalnum((unsigned char)code[i])) return 0;
    }
    
    return 1;
}

/**
 * Returns the price for a given product code.
 * Uses input validation to prevent injection attacks.
 * Returns -1.0 on error.
 */
double get_price_for_product_code(const char* product_code) {
    /* Input validation: check NULL and empty */
    if (product_code == NULL || product_code[0] == '\\0') {
        return -1.0;
    }
    
    /* Create a safe copy for trimming */
    char code[MAX_PRODUCT_CODE_LEN + 1];
    strncpy(code, product_code, MAX_PRODUCT_CODE_LEN);
    code[MAX_PRODUCT_CODE_LEN] = '\\0';
    trim(code);
    
    /* Input validation: check format */
    if (!is_valid_product_code(code)) {
        return -1.0;
    }
    
    /* Search for product in database */
    for (size_t i = 0; i < DB_SIZE; i++) {
        if (strcmp(code, PRICE_DATABASE[i].code) == 0) {
            return PRICE_DATABASE[i].price;
        }
    }
    
    /* Product not found */
    return -1.0;
}

/* Simple YAML parser for key: value format (safe, no code execution) */
static int parse_yaml_value(const char* yaml, const char* key, char* value, size_t value_size) {
    if (yaml == NULL || key == NULL || value == NULL || value_size == 0) {
        return 0;
    }
    
    /* Initialize value to empty */
    value[0] = '\\0';
    
    /* Search for key in YAML */
    char search_key[128];
    snprintf(search_key, sizeof(search_key), "%s:", key);
    
    const char* line_start = yaml;
    while (line_start != NULL && *line_start != '\\0') {
        /* Find end of line */
        const char* line_end = strchr(line_start, '\\n');
        size_t line_len = line_end ? (size_t)(line_end - line_start) : strlen(line_start);
        
        /* Check if line contains our key */
        const char* key_pos = strstr(line_start, search_key);
        if (key_pos != NULL && key_pos < line_start + line_len) {
            /* Extract value after colon */
            const char* val_start = key_pos + strlen(search_key);
            while (*val_start == ' ' || *val_start == '\\t') val_start++;
            
            /* Copy value */
            size_t copy_len = line_end ? (size_t)(line_end - val_start) : strlen(val_start);
            if (copy_len >= value_size) copy_len = value_size - 1;
            
            memcpy(value, val_start, copy_len);
            value[copy_len] = '\\0';
            trim(value);
            
            /* Remove quotes if present */
            size_t vlen = strlen(value);
            if (vlen >= 2 && (value[0] == '"' || value[0] == '\\'')) {
                memmove(value, value + 1, vlen - 2);
                value[vlen - 2] = '\\0';
            }
            
            return 1;
        }
        
        /* Move to next line */
        line_start = line_end ? line_end + 1 : NULL;
    }
    
    return 0;
}

/**
 * Calculates total price from YAML payload.
 * Uses simple parser to prevent arbitrary code execution.
 * Returns allocated string (caller must free) or NULL on error.
 */
char* calculate_price(const char* yaml_payload) {
    char* response = NULL;
    char product_code[MAX_PRODUCT_CODE_LEN + 1];
    char quantity_str[32];
    
    /* Allocate response buffer */
    response = (char*)malloc(MAX_RESPONSE_SIZE);
    if (response == NULL) {
        return NULL;
    }
    
    /* Input validation: check NULL */
    if (yaml_payload == NULL) {
        snprintf(response, MAX_RESPONSE_SIZE, "status: error\\nmessage: Payload cannot be null\\n");
        return response;
    }
    
    /* Input validation: check size limits */
    size_t payload_len = strlen(yaml_payload);
    if (payload_len > MAX_PAYLOAD_SIZE) {
        snprintf(response, MAX_RESPONSE_SIZE, "status: error\\nmessage: Payload exceeds maximum size\\n");
        return response;
    }
    
    /* Parse product_code */
    if (!parse_yaml_value(yaml_payload, "product_code", product_code, sizeof(product_code))) {
        snprintf(response, MAX_RESPONSE_SIZE, "status: error\\nmessage: Missing or invalid product_code\\n");
        return response;
    }
    
    /* Parse quantity */
    if (!parse_yaml_value(yaml_payload, "quantity", quantity_str, sizeof(quantity_str))) {
        snprintf(response, MAX_RESPONSE_SIZE, "status: error\\nmessage: Missing quantity\\n");
        return response;
    }
    
    /* Convert quantity to integer with validation */
    char* endptr;
    long quantity_long = strtol(quantity_str, &endptr, 10);
    
    /* Check conversion success */
    if (*endptr != '\\0' || quantity_str == endptr) {
        snprintf(response, MAX_RESPONSE_SIZE, "status: error\\nmessage: Invalid quantity format\\n");
        return response;
    }
    
    /* Validate quantity range */
    if (quantity_long <= 0 || quantity_long > MAX_QUANTITY) {
        snprintf(response, MAX_RESPONSE_SIZE, "status: error\\nmessage: Quantity out of valid range\\n");
        return response;
    }
    
    int quantity = (int)quantity_long;
    
    /* Get price for product code */
    double unit_price = get_price_for_product_code(product_code);
    if (unit_price < 0.0) {
        snprintf(response, MAX_RESPONSE_SIZE, "status: error\\nmessage: Invalid input\\n");
        return response;
    }
    
    /* Calculate total with overflow check */
    double total_price = unit_price * quantity;
    
    /* Check for overflow */
    if (isinf(total_price) || isnan(total_price)) {
        snprintf(response, MAX_RESPONSE_SIZE, "status: error\\nmessage: Calculation overflow\\n");
        return response;
    }
    
    /* Create success response */
    snprintf(response, MAX_RESPONSE_SIZE, "status: success\\ntotal_price: %.2f\\n", total_price);
    
    return response;
}

int main(void) {
    char* result = NULL;
    
    printf("Test Case 1: Valid input\\n");
    result = calculate_price("product_code: PROD001\\nquantity: 5");
    if (result) {
        printf("%s\\n", result);
        free(result);
    }
    
    printf("\\nTest Case 2: Different product\\n");
    result = calculate_price("product_code: PROD003\\nquantity: 10");
    if (result) {
        printf("%s\\n", result);
        free(result);
    }
    
    printf("\\nTest Case 3: Invalid product code\\n");
    result = calculate_price("product_code: INVALID\\nquantity: 5");
    if (result) {
        printf("%s\\n", result);
        free(result);
    }
    
    printf("\\nTest Case 4: Negative quantity\\n");
    result = calculate_price("product_code: PROD002\\nquantity: -5");
    if (result) {
        printf("%s\\n", result);
        free(result);
    }
    
    printf("\\nTest Case 5: Missing quantity\\n");
    result = calculate_price("product_code: PROD001");
    if (result) {
        printf("%s\\n", result);
        free(result);
    }
    
    return 0;
}
