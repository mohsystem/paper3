#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// Max length for result buffer
#define RESULT_BUFFER_SIZE 256

typedef struct {
    const char* id;
    const char* name;
    double price;
} Product;

static const Product PRODUCT_TABLE[] = {
    {"A123", "Laptop", 999.99},
    {"B456", "Mouse", 25.50},
    {"C789", "Keyboard", 75.00},
    {NULL, NULL, 0.0} // Sentinel to mark the end of the array
};

/**
 * @brief Inquires the product table for product details.
 * 
 * @param product_id The ID of the product to look for.
 * @param result_buffer A caller-provided buffer to store the result string.
 * @param buffer_size The size of the result_buffer.
 */
void getProductDetails(const char* product_id, char* result_buffer, size_t buffer_size) {
    // Rule #4: Ensure buffer is valid before writing
    if (result_buffer == NULL || buffer_size == 0) {
        return; // Cannot write to an invalid buffer
    }

    // Rule #3: Validate input. Treat all inputs as untrusted.
    if (product_id == NULL || product_id[0] == '\0') {
        snprintf(result_buffer, buffer_size, "Error: Product ID cannot be null or empty.");
        return;
    }

    bool found = false;
    for (int i = 0; PRODUCT_TABLE[i].id != NULL; ++i) {
        if (strcmp(product_id, PRODUCT_TABLE[i].id) == 0) {
            // Rule #1 & #4: Use safe formatting and check buffer boundaries with snprintf
            snprintf(result_buffer, buffer_size, "Product: %s, Price: $%.2f",
                     PRODUCT_TABLE[i].name, PRODUCT_TABLE[i].price);
            found = true;
            break;
        }
    }

    if (!found) {
        snprintf(result_buffer, buffer_size, "Product with ID '%.50s' not found.", product_id);
    }
}

int main() {
    printf("--- Product Inquiry System ---\n");

    const char* testCases[] = {
        "A123",       // Test Case 1: Valid, existing product ID
        "C789",       // Test Case 2: Another valid, existing product ID
        "X999",       // Test Case 3: Valid format, but non-existent ID
        "",           // Test Case 4: Invalid empty string
        NULL          // Test Case 5: Invalid null input
    };
    
    char result[RESULT_BUFFER_SIZE];

    for (int i = 0; i < 5; ++i) {
        printf("Test Case %d (Input: \"%s\"):\n", i + 1, testCases[i] == NULL ? "NULL" : testCases[i]);
        // Rule #4: Initialize buffer before use
        result[0] = '\0'; 
        getProductDetails(testCases[i], result, RESULT_BUFFER_SIZE);
        printf("  Result: %s\n\n", result);
    }

    return 0;
}