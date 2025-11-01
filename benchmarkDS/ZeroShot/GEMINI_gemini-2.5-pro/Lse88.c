// libyaml is a third-party library. To compile this code, you need to have it installed.
// For example, on Ubuntu: sudo apt-get install libyaml-dev
// Compile with: gcc -o Lse88 Lse88.c -lyaml
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <yaml.h>

// Simple price lookup table
double get_price(const char* product_code) {
    if (strcmp(product_code, "PROD1") == 0) return 10.50;
    if (strcmp(product_code, "PROD2") == 0) return 25.00;
    if (strcmp(product_code, "PROD3") == 0) return 5.75;
    return -1.0; // Not found
}

/**
 * @brief Calculates the total price based on a YAML payload.
 *
 * This function uses the low-level libyaml C library. libyaml is a parser,
 * not an interpreter, so it is inherently safe from arbitrary code execution.
 * Security depends on the careful handling of parsing events and memory
 * management in this function itself.
 *
 * @param yaml_payload A string containing the YAML payload.
 * @return A dynamically allocated string with the result or an error message.
 *         The caller is responsible for freeing this string.
 */
char* calculate_price(const char* yaml_payload) {
    yaml_parser_t parser;
    yaml_event_t event;
    char* result = NULL;
    char* out_buffer = (char*)malloc(100);
    if (!out_buffer) return strdup("Error: Memory allocation failed.");

    char* found_code = NULL;
    int found_qty = 0;
    int expecting_value_for_code = 0;
    int expecting_value_for_qty = 0;
    
    if (!yaml_parser_initialize(&parser)) {
        snprintf(out_buffer, 100, "Error: Failed to initialize YAML parser.");
        return out_buffer;
    }

    yaml_parser_set_input_string(&parser, (const unsigned char*)yaml_payload, strlen(yaml_payload));

    int done = 0;
    while (!done) {
        if (!yaml_parser_parse(&parser, &event)) {
            snprintf(out_buffer, 100, "Error: Invalid YAML format at line %zu.", parser.problem_mark.line + 1);
            result = strdup(out_buffer);
            goto cleanup;
        }

        switch (event.type) {
            case YAML_SCALAR_EVENT:
                if (expecting_value_for_code) {
                    found_code = strdup((char*)event.data.scalar.value);
                    expecting_value_for_code = 0;
                } else if (expecting_value_for_qty) {
                    found_qty = atoi((char*)event.data.scalar.value);
                    if (found_qty == 0 && strcmp((char*)event.data.scalar.value, "0") != 0) {
                         snprintf(out_buffer, 100, "Error: 'quantity' must be a valid number.");
                         result = strdup(out_buffer);
                         goto cleanup;
                    }
                    expecting_value_for_qty = 0;
                } else {
                    if (strcmp((char*)event.data.scalar.value, "product_code") == 0) {
                        expecting_value_for_code = 1;
                    } else if (strcmp((char*)event.data.scalar.value, "quantity") == 0) {
                        expecting_value_for_qty = 1;
                    }
                }
                break;
            case YAML_STREAM_END_EVENT:
                done = 1;
                break;
            // We only care about scalars and the end of the stream for this simple case
            case YAML_STREAM_START_EVENT:
            case YAML_DOCUMENT_START_EVENT:
            case YAML_MAPPING_START_EVENT:
            case YAML_MAPPING_END_EVENT:
            case YAML_DOCUMENT_END_EVENT:
                break; // Ignore these events
            default:
                snprintf(out_buffer, 100, "Error: Unsupported YAML structure.");
                result = strdup(out_buffer);
                goto cleanup;
        }
        yaml_event_delete(&event);
    }

    if (!found_code || found_qty == 0) {
        snprintf(out_buffer, 100, "Error: Missing 'product_code' or 'quantity' in YAML payload.");
        result = strdup(out_buffer);
    } else if (found_qty <= 0) {
        snprintf(out_buffer, 100, "Error: Quantity must be positive.");
        result = strdup(out_buffer);
    } else {
        double price = get_price(found_code);
        if (price < 0) {
            snprintf(out_buffer, 100, "Error: Product code not found.");
            result = strdup(out_buffer);
        } else {
            double total_price = price * found_qty;
            snprintf(out_buffer, 100, "%.2f", total_price);
            result = strdup(out_buffer);
        }
    }

cleanup:
    free(found_code);
    free(out_buffer);
    yaml_event_delete(&event); // Free last event if loop was broken
    yaml_parser_delete(&parser);
    return result;
}

int main() {
    const char* test_cases[] = {
        // Test Case 1: Valid input
        "product_code: \"PROD1\"\nquantity: 10",
        // Test Case 2: Unknown product code
        "product_code: \"PROD99\"\nquantity: 5",
        // Test Case 3: Malformed YAML (invalid syntax)
        "product_code: \"PROD2\" quantity: 2",
        // Test Case 4: Missing 'quantity' key
        "product_code: \"PROD3\"",
        // Test Case 5: Invalid quantity type
        "product_code: \"PROD1\"\nquantity: \"five\""
    };
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);

    for (int i = 0; i < num_tests; ++i) {
        printf("Test Case %d:\n", i + 1);
        printf("Input:\n---\n%s\n---\n", test_cases[i]);
        char* result = calculate_price(test_cases[i]);
        printf("Output: %s\n\n", result);
        free(result); // IMPORTANT: Free the memory allocated by the function
    }

    return 0;
}