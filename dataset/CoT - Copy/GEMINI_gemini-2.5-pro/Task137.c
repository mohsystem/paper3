#include <stdio.h>
#include <string.h>

// Struct to represent a product. Using a fixed-size char array for the name.
typedef struct {
    char name[50];
    double price;
} Product;

// Struct to map a product ID to product data.
typedef struct {
    int id;
    Product product;
} ProductEntry;

// Our in-memory product table. 'static const' makes it read-only
// and internal to this file.
static const ProductEntry productTable[] = {
    {101, {"Laptop", 1200.50}},
    {102, {"Mouse", 25.00}},
    {103, {"Keyboard", 75.75}},
    {104, {"Monitor", 300.00}},
    {105, {"Webcam", 50.25}}
};

// Calculate the number of entries in the table at compile time.
static const int numProducts = sizeof(productTable) / sizeof(productTable[0]);

/**
 * @brief Inquires the product table for a product's details based on its ID.
 * This is a secure C implementation that avoids buffer overflows.
 *
 * @param productId The ID of the product to look up.
 * @param resultBuffer A pointer to the character buffer to write the result into.
 * @param bufferSize The total size of the resultBuffer.
 */
void getProductDetails(int productId, char* resultBuffer, size_t bufferSize) {
    // Input validation: Ensure the buffer is not NULL and has space.
    if (resultBuffer == NULL || bufferSize == 0) {
        return; 
    }

    int found = 0;
    for (int i = 0; i < numProducts; ++i) {
        if (productTable[i].id == productId) {
            // Use snprintf for safe, bounded string formatting to prevent buffer overflows.
            snprintf(resultBuffer, bufferSize, "Product ID: %d, Name: %s, Price: $%.2f",
                     productId, productTable[i].product.name, productTable[i].product.price);
            found = 1;
            break;
        }
    }

    if (!found) {
        snprintf(resultBuffer, bufferSize, "Product with ID %d not found.", productId);
    }
    
    // Ensure the buffer is null-terminated, even if snprintf truncates.
    resultBuffer[bufferSize - 1] = '\0';
}

int main() {
    printf("--- Product Inquiry System ---\n");

    // Test cases
    int testCases[] = {101, 104, 105, 999, 103};
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    // A buffer to hold the results from the function.
    char result[256];

    for (int i = 0; i < numTestCases; i++) {
        printf("Test Case %d: Searching for product ID %d\n", i + 1, testCases[i]);
        getProductDetails(testCases[i], result, sizeof(result));
        printf("Result: %s\n\n", result);
    }

    return 0;
}