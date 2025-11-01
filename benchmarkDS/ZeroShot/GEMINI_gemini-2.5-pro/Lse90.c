#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// This code is a simplified demonstration.
// For a production environment, a proper YAML parsing library like libyaml is recommended.
// Using string manipulation is insecure and brittle, but included here for a self-contained example
// as integrating libyaml is complex for a single file. This approach is NOT secure.
// A secure implementation would use a robust parser and careful memory management.

typedef struct {
    const char* code;
    double price;
} Product;

static const Product product_prices[] = {
    {"PROD001", 10.50},
    {"PROD002", 25.00},
    {"PROD003", 5.75},
    {"PROD004", 150.00},
    {NULL, 0.0} // Sentinel
};

/**
 * @brief Returns the price for a given product code.
 * @param product_code The code of the product.
 * @return The price of the product, or -1.0 if not found.
 */
double get_price_for_product_code(const char* product_code) {
    if (!product_code) return -1.0;
    for (int i = 0; product_prices[i].code != NULL; ++i) {
        if (strcmp(product_prices[i].code, product_code) == 0) {
            return product_prices[i].price;
        }
    }
    return -1.0;
}

// A helper to safely extract values. This is NOT a real YAML parser.
char* get_value_from_payload(const char* payload, const char* key) {
    const char* key_pos = strstr(payload, key);
    if (!key_pos) return NULL;

    const char* value_start = key_pos + strlen(key);
    while (*value_start == ' ' || *value_start == ':') {
        value_start++;
    }

    const char* value_end = value_start;
    while (*value_end && *value_end != '\n' && *value_end != '\r') {
        value_end++;
    }

    size_t len = value_end - value_start;
    char* value = (char*)malloc(len + 1);
    if (!value) return NULL;
    
    strncpy(value, value_start, len);
    value[len] = '\0';
    return value;
}

/**
 * @brief Simulates a web route that takes a YAML payload, calculates the total price,
 * and returns the result as a YAML payload.
 *
 * @param yaml_payload A string containing the YAML request.
 * @return A new YAML string with the total price or an error. The caller must free this string.
 */
char* calculate_price(const char* yaml_payload) {
    char* result_buffer = (char*)malloc(256 * sizeof(char));
    if (!result_buffer) {
        return NULL;
    }

    char* product_code_str = get_value_from_payload(yaml_payload, "product_code");
    char* quantity_str = get_value_from_payload(yaml_payload, "quantity");

    if (!product_code_str || !quantity_str) {
        snprintf(result_buffer, 256, "error: \"Invalid payload: 'product_code' and 'quantity' are required.\"\n");
    } else {
        // Use strtol for safer integer conversion
        char *endptr;
        long quantity_long = strtol(quantity_str, &endptr, 10);
        
        // Check if conversion was successful and value is valid
        if (*endptr != '\0' || quantity_long <= 0 || quantity_long > 2147483647) {
             snprintf(result_buffer, 256, "error: \"Invalid or non-positive quantity.\"\n");
        } else {
            int quantity = (int)quantity_long;
            double price = get_price_for_product_code(product_code_str);

            if (price < 0) {
                snprintf(result_buffer, 256, "error: \"Product code not found.\"\n");
            } else {
                double total_price = price * quantity;
                snprintf(result_buffer, 256, "total_price: %.2f\n", total_price);
            }
        }
    }
    
    free(product_code_str);
    free(quantity_str);
    
    return result_buffer;
}

int main() {
    const char* test_cases[] = {
        // Test Case 1: Valid request
        "product_code: PROD001\nquantity: 10",
        // Test Case 2: Another valid request
        "product_code: PROD004\nquantity: 2",
        // Test Case 3: Product code not found
        "product_code: PROD999\nquantity: 5",
        // Test Case 4: Invalid payload (missing quantity)
        "product_code: PROD002",
        // Test Case 5: Malformed quantity
        "product_code: PROD003\nquantity: three"
    };

    printf("--- Running C Test Cases ---\n");
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    for (int i = 0; i < num_tests; i++) {
        printf("Test Case %d:\n", i + 1);
        printf("Input:\n---\n%s\n---\n", test_cases[i]);
        char* result = calculate_price(test_cases[i]);
        if (result) {
            printf("Output:\n%s\n", result);
            free(result); // Free the memory allocated by the function
        } else {
            printf("Output: Memory allocation failed.\n");
        }
    }

    return 0;
}