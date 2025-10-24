#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_BUFFER_SIZE 256

typedef struct {
    const char* id;
    const char* name;
    double price;
    const char* description;
} Product;

// In-memory "database" of products using a static constant array of structs.
static const Product productTable[] = {
    {"PROD101", "Laptop", 999.99, "High-performance laptop"},
    {"PROD102", "Mouse", 25.50, "Ergonomic wireless mouse"},
    {"PROD103", "Keyboard", 75.00, "Mechanical gaming keyboard"},
    {"PROD104", "Monitor", 300.00, "27-inch 4K UHD monitor"},
    {NULL, NULL, 0.0, NULL} // Sentinel value to mark the end of the array
};

// Helper function to check if a string is NULL, empty, or just whitespace
int isStringBlank(const char* s) {
    if (s == NULL) {
        return 1;
    }
    while (*s) {
        if (!isspace((unsigned char)*s)) {
            return 0; // Found a non-whitespace character
        }
        s++;
    }
    return 1; // String is all whitespace or empty
}

/**
 * @brief Inquires the product table for product details in a secure manner.
 * This function is secure because:
 * 1. Input is validated for NULL and blank content.
 * 2. It searches a read-only, in-memory data structure.
 * 3. It uses `strcmp` for comparison, which is safe as input is not used
 *    to modify memory or construct queries.
 * 4. It uses `snprintf` for formatting output into a fixed-size buffer,
 *    which prevents buffer overflows.
 * 
 * @param productId The ID of the product to look up.
 * @return A constant string with product details or an error message.
 *         NOTE: The returned pointer for a found product points to a static buffer
 *         and should NOT be freed. It will be overwritten on subsequent calls.
 */
const char* getProductDetails(const char* productId) {
    // A static buffer to hold the formatted result string.
    // This avoids dynamic allocation, but is not thread-safe.
    static char resultBuffer[MAX_BUFFER_SIZE];

    // 1. Input Validation: Check for NULL, empty, or whitespace-only strings.
    if (isStringBlank(productId)) {
        return "Error: Product ID cannot be null or empty.";
    }

    // 2. Data Lookup: Linearly scan the product table.
    for (int i = 0; productTable[i].id != NULL; ++i) {
        if (strcmp(productId, productTable[i].id) == 0) {
            const Product* product = &productTable[i];
            
            // 3. Result Formatting: Use snprintf to prevent buffer overflows.
            snprintf(resultBuffer, MAX_BUFFER_SIZE,
                     "Name: %s, Price: %.2f, Description: %s",
                     product->name, product->price, product->description);
            
            return resultBuffer;
        }
    }

    // 4. Handle not found case.
    return "Product not found.";
}

int main() {
    printf("--- Product Inquiry System ---\n");

    // Test Case 1: Valid product ID
    const char* test1 = "PROD101";
    printf("Querying for ID: '%s'\n", test1);
    printf("Result: %s\n\n", getProductDetails(test1));

    // Test Case 2: Another valid product ID
    const char* test2 = "PROD103";
    printf("Querying for ID: '%s'\n", test2);
    printf("Result: %s\n\n", getProductDetails(test2));

    // Test Case 3: Non-existent product ID
    const char* test3 = "PROD999";
    printf("Querying for ID: '%s'\n", test3);
    printf("Result: %s\n\n", getProductDetails(test3));

    // Test Case 4: Whitespace string input
    const char* test4 = "   ";
    printf("Querying for ID: '%s'\n", test4);
    printf("Result: %s\n\n", getProductDetails(test4));

    // Test Case 5: NULL input
    const char* test5 = NULL;
    printf("Querying for ID: '(null)'\n");
    printf("Result: %s\n\n", getProductDetails(test5));
    
    return 0;
}