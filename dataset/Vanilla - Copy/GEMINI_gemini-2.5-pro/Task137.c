#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// A struct to represent a product
typedef struct {
    int id;
    char name[50];
    double price;
    int quantity;
} Product;

// Global product table (simulating a database)
const Product productTable[] = {
    {101, "Laptop", 1200.50, 10},
    {102, "Mouse", 25.00, 150},
    {103, "Keyboard", 75.99, 80},
    {104, "Monitor", 300.00, 45},
    {105, "USB Hub", 15.50, 200}
};
const int numProducts = sizeof(productTable) / sizeof(Product);

/**
 * Inquires the product table for a product's details by its name.
 * @param productName The name of the product to search for.
 * @return A dynamically allocated string with the product details or a "not found" message.
 *         The caller is responsible for freeing this memory.
 */
char* getProductDetails(const char* productName) {
    for (int i = 0; i < numProducts; ++i) {
        if (strcmp(productTable[i].name, productName) == 0) {
            // Allocate buffer for the result string. 200 chars should be plenty.
            char* result = (char*)malloc(200 * sizeof(char));
            if (result == NULL) {
                return NULL; // Memory allocation failed
            }
            sprintf(result, "ID: %d, Name: %s, Price: %.2f, Quantity: %d",
                    productTable[i].id, productTable[i].name,
                    productTable[i].price, productTable[i].quantity);
            return result;
        }
    }

    // Product not found, return a "not found" message
    char* notFoundMsg = (char*)malloc(20 * sizeof(char));
    if (notFoundMsg == NULL) {
        return NULL; // Memory allocation failed
    }
    strcpy(notFoundMsg, "Product not found.");
    return notFoundMsg;
}

int main() {
    // 5 test cases
    const char* testCases[] = {"Laptop", "Monitor", "Mouse", "Webcam", "USB Hub"};

    printf("--- C Product Inquiry System ---\n");
    for (int i = 0; i < 5; ++i) {
        printf("Querying for: %s\n", testCases[i]);
        char* details = getProductDetails(testCases[i]);
        if (details != NULL) {
            printf("Result: %s\n\n", details);
            // Free the dynamically allocated memory returned by the function
            free(details);
        } else {
            printf("Error: Memory allocation failed.\n\n");
        }
    }

    return 0;
}