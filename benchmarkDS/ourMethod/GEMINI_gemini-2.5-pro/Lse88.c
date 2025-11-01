// Note: This code requires the libyaml library.
// On Debian/Ubuntu: sudo apt-get install libyaml-dev
// Compilation: gcc -std=c99 -o c_app your_file_name.c -lyaml

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <yaml.h>

#define MAX_KEY_LENGTH 64
#define MAX_RESULT_LENGTH 256
#define MAX_QUANTITY 1000000

typedef struct {
    char product_code[MAX_KEY_LENGTH];
    double price;
} product_price_t;

static const product_price_t PRODUCT_PRICES[] = {
    {"PROD_A", 19.99},
    {"PROD_B", 250.00},
    {"PROD_C", 0.50}
};
static const int NUM_PRODUCTS = sizeof(PRODUCT_PRICES) / sizeof(PRODUCT_PRICES[0]);

// Helper function to find the price for a given product code.
double get_product_price(const char* product_code) {
    if (product_code == NULL) return -1.0;
    for (int i = 0; i < NUM_PRODUCTS; ++i) {
        if (strncmp(product_code, PRODUCT_PRICES[i].product_code, MAX_KEY_LENGTH) == 0) {
            return PRODUCT_PRICES[i].price;
        }
    }
    return -1.0; // Not found
}

// Helper to create a heap-allocated string for returning.
char* create_return_string(const char* message) {
    if (message == NULL) return NULL;
    size_t len = strlen(message);
    char* str = malloc(len + 1);
    if (str) {
        memcpy(str, message, len);
        str[len] = '\0';
    }
    return str;
}

/**
 * Parses a YAML payload, calculates the total price, and returns it as a string.
 * The caller is responsible for freeing the returned string.
 *
 * @param yamlPayload A constant character pointer to the YAML payload string.
 * @return A heap-allocated string with the result or an error message. NULL on memory allocation failure.
 */
char* calculate_price(const char* yamlPayload) {
    if (yamlPayload == NULL || *yamlPayload == '\0') {
        return create_return_string("Error: YAML payload cannot be null or empty.");
    }
    
    yaml_parser_t parser;
    yaml_event_t event;
    
    char product_code[MAX_KEY_LENGTH] = {0};
    long long quantity = -1;
    int has_product_code = 0;
    int has_quantity = 0;
    int error = 0;
    char error_message[MAX_RESULT_LENGTH] = "Error: Failed to parse YAML.";

    enum { STATE_START, STATE_KEY, STATE_VALUE_PROD, STATE_VALUE_QTY } state = STATE_START;
    char last_key[MAX_KEY_LENGTH] = {0};

    if (!yaml_parser_initialize(&parser)) {
        return create_return_string("Error: Failed to initialize YAML parser.");
    }
    
    yaml_parser_set_input_string(&parser, (const unsigned char*)yamlPayload, strlen(yamlPayload));

    int done = 0;
    while (!done && !error) {
        if (!yaml_parser_parse(&parser, &event)) {
            // Do not provide detailed parser errors to the client.
            error = 1;
            break;
        }

        switch (event.type) {
            case YAML_MAPPING_START_EVENT: state = STATE_KEY; break;
            case YAML_SCALAR_EVENT:
                if (state == STATE_KEY) {
                    strncpy(last_key, (char*)event.data.scalar.value, MAX_KEY_LENGTH - 1);
                    last_key[MAX_KEY_LENGTH - 1] = '\0';
                    if (strcmp(last_key, "product_code") == 0) state = STATE_VALUE_PROD;
                    else if (strcmp(last_key, "quantity") == 0) state = STATE_VALUE_QTY;
                    else state = STATE_KEY; // Ignore unknown keys
                } else if (state == STATE_VALUE_PROD) {
                    strncpy(product_code, (char*)event.data.scalar.value, MAX_KEY_LENGTH - 1);
                    product_code[MAX_KEY_LENGTH - 1] = '\0';
                    has_product_code = 1;
                    state = STATE_KEY;
                } else if (state == STATE_VALUE_QTY) {
                    char* endptr;
                    quantity = strtoll((char*)event.data.scalar.value, &endptr, 10);
                    if (*endptr != '\0') {
                        snprintf(error_message, sizeof(error_message), "Error: 'quantity' must be an integer.");
                        error = 1;
                    } else {
                        has_quantity = 1;
                    }
                    state = STATE_KEY;
                }
                break;
            case YAML_STREAM_END_EVENT: done = 1; break;
            default: break; // Ignore other events
        }
        yaml_event_delete(&event);
    }
    
    yaml_parser_delete(&parser);

    if (error) return create_return_string(error_message);
    
    if (!has_product_code) return create_return_string("Error: Missing 'product_code'.");
    if (strlen(product_code) == 0) return create_return_string("Error: 'product_code' cannot be empty.");
    if (!has_quantity) return create_return_string("Error: Missing 'quantity'.");
    
    if (quantity < 0) return create_return_string("Error: Quantity cannot be negative.");
    if (quantity > MAX_QUANTITY) {
        char msg[100];
        snprintf(msg, sizeof(msg), "Error: Quantity exceeds maximum limit of %d", MAX_QUANTITY);
        return create_return_string(msg);
    }

    double price = get_product_price(product_code);
    if (price < 0.0) return create_return_string("Error: Product not found.");
    
    double total_price = price * (double)quantity;

    char* result_str = malloc(MAX_RESULT_LENGTH);
    if (!result_str) return NULL; // Memory allocation failed
    snprintf(result_str, MAX_RESULT_LENGTH, "%.2f", total_price);

    return result_str;
}

void run_test_case(int caseNum, const char* input) {
    printf("--- Test Case %d ---\n", caseNum);
    printf("Input:\n%s\n", input);
    char* result = calculate_price(input);
    if (result) {
        printf("Output: %s\n", result);
        free(result);
    } else {
        printf("Output: (null) Error: Memory allocation failed.\n");
    }
    printf("\n");
}

int main() {
    const char* test1 = "product_code: 'PROD_A'\nquantity: 10";
    run_test_case(1, test1);

    const char* test2 = "product_code: 'PROD_B\n  quantity: 5";
    run_test_case(2, test2);

    const char* test3 = "product_code: 'PROD_X'\nquantity: 100";
    run_test_case(3, test3);

    const char* test4 = "product_code: 'PROD_C'\nquantity: -5";
    run_test_case(4, test4);

    const char* test5 = "product_code: 'PROD_A'\nquantity: 'twenty'";
    run_test_case(5, test5);

    return 0;
}