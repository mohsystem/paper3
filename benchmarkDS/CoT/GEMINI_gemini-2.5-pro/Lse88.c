#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Note: This code requires the libyaml library.
// You need to install it and link against it during compilation, e.g.:
// gcc -o Lse88 Lse88.c -lyaml
#include <yaml.h>

// Using a simple struct and an array to simulate a map for product prices
typedef struct {
    const char* code;
    double price;
} Product;

static const Product PRODUCT_PRICES[] = {
    {"PROD001", 10.50},
    {"PROD002", 25.00},
    {"PROD003", 5.75},
    {NULL, 0.0} // Sentinel value
};

// Helper function to look up price
double get_price(const char* product_code) {
    if (product_code == NULL) return -1.0;
    for (int i = 0; PRODUCT_PRICES[i].code != NULL; ++i) {
        if (strcmp(PRODUCT_PRICES[i].code, product_code) == 0) {
            return PRODUCT_PRICES[i].price;
        }
    }
    return -1.0; // Not found
}

/**
 * @brief Parses a YAML payload, calculates the total price, and returns it as a string.
 *
 * @param yamlPayload A string containing the YAML payload with product_code and quantity.
 * @return A dynamically allocated string with the result, which must be freed by the caller.
 *         Returns an error message on failure.
 */
char* calculate_price(const char* yamlPayload) {
    yaml_parser_t parser;
    yaml_event_t event;
    
    char* result_buffer = (char*)malloc(256 * sizeof(char));
    if (!result_buffer) return NULL;

    // Use a simple state machine for parsing
    enum ParseState { KEY, VALUE_CODE, VALUE_QTY };
    enum ParseState state = KEY;
    
    char* product_code = NULL;
    int quantity = 0;
    int found_code = 0;
    int found_qty = 0;

    // Initialize parser
    if (!yaml_parser_initialize(&parser)) {
        snprintf(result_buffer, 256, "Error: Failed to initialize YAML parser.");
        return result_buffer;
    }

    // Set input string
    yaml_parser_set_input_string(&parser, (const unsigned char*)yamlPayload, strlen(yamlPayload));

    // Parsing loop
    int done = 0;
    while (!done) {
        if (!yaml_parser_parse(&parser, &event)) {
            snprintf(result_buffer, 256, "Error: Invalid YAML format at line %zu.", parser.problem_mark.line);
            goto cleanup;
        }

        if (event.type == YAML_SCALAR_EVENT) {
            char* value = (char*)event.data.scalar.value;
            if (state == KEY) {
                if (strcmp(value, "product_code") == 0) {
                    state = VALUE_CODE;
                } else if (strcmp(value, "quantity") == 0) {
                    state = VALUE_QTY;
                }
            } else if (state == VALUE_CODE) {
                product_code = strdup(value);
                found_code = 1;
                state = KEY;
            } else if (state == VALUE_QTY) {
                char* end;
                long val = strtol(value, &end, 10);
                // Check if conversion was successful and the whole string was consumed
                if (end != value && *end == '\0') {
                    quantity = (int)val;
                    found_qty = 1;
                } else {
                    snprintf(result_buffer, 256, "Error: 'quantity' must be an integer.");
                    goto cleanup;
                }
                state = KEY;
            }
        }
        
        done = (event.type == YAML_STREAM_END_EVENT);
        yaml_event_delete(&event);
    }
    
    if (!found_code || !found_qty) {
        snprintf(result_buffer, 256, "Error: Missing 'product_code' or 'quantity' in payload.");
        goto cleanup;
    }

    if (quantity <= 0) {
        snprintf(result_buffer, 256, "Error: Quantity must be a positive integer.");
        goto cleanup;
    }

    double price = get_price(product_code);
    if (price < 0) {
        snprintf(result_buffer, 256, "Error: Product code not found.");
        goto cleanup;
    }

    double total_price = price * quantity;
    snprintf(result_buffer, 256, "%.2f", total_price);

cleanup:
    if (product_code) free(product_code);
    yaml_parser_delete(&parser);
    return result_buffer;
}

// In C, a class name like Lse88 is not used. We'll use a main function.
int main() {
    const char* testCases[] = {
        // 1. Valid case
        "product_code: 'PROD002'\nquantity: 3",
        // 2. Product not found
        "product_code: 'PROD999'\nquantity: 1",
        // 3. Invalid YAML format (bad indentation)
        "product_code: 'PROD001'\n  quantity: 5",
        // 4. Missing key ('quantity')
        "product_code: 'PROD003'",
        // 5. Invalid data type for quantity
        "product_code: 'PROD001'\nquantity: 'two'"
    };
    int num_tests = sizeof(testCases) / sizeof(testCases[0]);

    printf("--- C Test Cases ---\n");
    for (int i = 0; i < num_tests; ++i) {
        printf("Test Case %d:\nInput:\n---\n%s\n---\n", i + 1, testCases[i]);
        char* result = calculate_price(testCases[i]);
        if (result) {
            printf("Output: %s\n\n", result);
            free(result); // IMPORTANT: Free the memory allocated by the function
        } else {
            printf("Output: Error: Memory allocation failed.\n\n");
        }
    }

    return 0;
}