#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char* code;
    double price;
} Product;

static const Product prices[] = {
    {"P101", 10.50},
    {"P102", 25.00},
    {"P103", 5.75},
    {NULL, 0.0} // Sentinel to mark the end of the array
};

double get_price(const char* code) {
    for (int i = 0; prices[i].code != NULL; i++) {
        if (strcmp(prices[i].code, code) == 0) {
            return prices[i].price;
        }
    }
    return -1.0; // Use a negative value to indicate "not found"
}

/**
 * Parses a simple YAML string to calculate the total price of a product.
 * The caller is responsible for freeing the returned string.
 *
 * @param yamlPayload A string containing product_code and quantity.
 * @return A dynamically allocated string with the total price or an error message.
 */
char* calculate_price(const char* yamlPayload) {
    char productCode[50] = "";
    int quantity = 0;
    
    int code_found = 0;
    int quantity_found = 0;

    const char* code_key = "product_code:";
    const char* qty_key = "quantity:";
    
    const char* code_line_start = strstr(yamlPayload, code_key);
    const char* qty_line_start = strstr(yamlPayload, qty_key);

    if (code_line_start) {
        // Try to parse with quotes, then without
        if (sscanf(code_line_start + strlen(code_key), " \"%49[^\"]\"", productCode) == 1) {
             code_found = 1;
        } else if (sscanf(code_line_start + strlen(code_key), " %49s", productCode) == 1) {
             code_found = 1;
        }
    }

    if (qty_line_start) {
        if (sscanf(qty_line_start + strlen(qty_key), " %d", &quantity) == 1) {
            quantity_found = 1;
        }
    }

    char* result = (char*)malloc(100 * sizeof(char));
    if (result == NULL) {
        return NULL; 
    }

    if (!code_found || !quantity_found) {
        if (qty_line_start && !quantity_found) {
             sprintf(result, "Error: Invalid quantity");
        } else {
             sprintf(result, "Error: Invalid format or missing keys");
        }
        return result;
    }
    
    double unitPrice = get_price(productCode);
    if (unitPrice < 0) {
        sprintf(result, "Error: Product not found");
        return result;
    }

    double totalPrice = unitPrice * quantity;
    sprintf(result, "%.2f", totalPrice);
    
    return result;
}

int main() {
    const char* testCases[] = {
        // Test Case 1: Standard valid case
        "product_code: \"P101\"\nquantity: 5",
        // Test Case 2: Another valid case (different order)
        "quantity: 10\nproduct_code: \"P103\"",
        // Test Case 3: Product not found
        "product_code: \"P999\"\nquantity: 2",
        // Test Case 4: Invalid quantity
        "product_code: \"P102\"\nquantity: abc",
        // Test Case 5: Malformed YAML (no colon on one line)
        "product_code \"P101\"\nquantity: 3"
    };

    printf("C Test Cases:\n");
    for (int i = 0; i < 5; ++i) {
        printf("Test Case %d:\n", i + 1);
        printf("Input:\n---\n%s\n---\n", testCases[i]);
        char* result = calculate_price(testCases[i]);
        if (result != NULL) {
            printf("Output: %s\n\n", result);
            free(result); // Free the allocated memory
        } else {
            printf("Output: NULL (memory allocation error)\n\n");
        }
    }

    return 0;
}