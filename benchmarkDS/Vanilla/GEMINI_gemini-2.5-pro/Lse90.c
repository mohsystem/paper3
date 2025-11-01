#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    char product_code[10];
    double price;
} ProductPrice;

static ProductPrice price_db[] = {
    {"P101", 10.50},
    {"P202", 25.00},
    {"P303", 5.75}
};
static const int DB_SIZE = sizeof(price_db) / sizeof(price_db[0]);

/**
 * Returns the price for a given product code.
 * @param product_code The code of the product.
 * @return The price of the product, or 0.0 if not found.
 */
double get_price_for_product_code(const char* product_code) {
    if (product_code == NULL) return 0.0;
    for (int i = 0; i < DB_SIZE; ++i) {
        if (strcmp(price_db[i].product_code, product_code) == 0) {
            return price_db[i].price;
        }
    }
    return 0.0;
}

/**
 * A very simple parser for a specific YAML format ("key: value").
 * It populates the provided buffers for product_code and quantity.
 * @param yaml_payload The YAML string to parse.
 * @param product_code Buffer to store the extracted product code.
 * @param quantity Pointer to store the extracted quantity.
 */
void parse_simple_yaml(const char* yaml_payload, char* product_code_out, int* quantity_out) {
    // Initialize defaults
    product_code_out[0] = '\0';
    *quantity_out = 0;
    
    char key[100];
    char value_str[100];
    const char* p = yaml_payload;
    
    while (sscanf(p, " %99[^:]: %99[^\n]", key, value_str) == 2) {
        if (strcmp(key, "product_code") == 0) {
            strcpy(product_code_out, value_str);
        } else if (strcmp(key, "quantity") == 0) {
            *quantity_out = atoi(value_str);
        }
        
        // Move pointer to the next line
        const char* next_line = strchr(p, '\n');
        if (next_line) {
            p = next_line + 1;
        } else {
            break;
        }
    }
}

/**
 * Simulates a web route that calculates the total price from a YAML payload.
 * @param yaml_payload A string in a simple YAML format containing product_code and quantity.
 * @return A dynamically allocated YAML string containing the total_price. The caller must free this string.
 */
char* calculate_price(const char* yaml_payload) {
    char product_code[50] = "";
    int quantity = 0;
    
    // Load "YAML" and extract data
    parse_simple_yaml(yaml_payload, product_code, &quantity);
    
    // Calculate the total price
    double price_per_unit = get_price_for_product_code(product_code);
    double total_price = price_per_unit * (double)quantity;
    
    // Allocate memory for the output string. e.g., "total_price: 12345.67\n"
    char* output_yaml = (char*)malloc(100 * sizeof(char));
    if (output_yaml == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }
    
    // Format the output YAML string
    sprintf(output_yaml, "total_price: %.2f\n", total_price);
    
    return output_yaml;
}

int main() {
    const char* test_cases[] = {
        "product_code: P101\nquantity: 5",    // Expected: 52.50
        "product_code: P202\nquantity: 1",    // Expected: 25.00
        "product_code: P303\nquantity: 100",  // Expected: 575.00
        "product_code: P999\nquantity: 10",   // Expected: 0.00 (product not found)
        "product_code: P101\nquantity: 0"     // Expected: 0.00
    };
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    
    for (int i = 0; i < num_tests; ++i) {
        printf("--- Test Case %d ---\n", i + 1);
        printf("Input YAML:\n---\n%s\n---\n", test_cases[i]);
        char* output_yaml = calculate_price(test_cases[i]);
        if (output_yaml) {
            printf("Output YAML:\n%s\n", output_yaml);
            free(output_yaml); // Free the memory allocated by calculate_price
        }
    }
    
    return 0;
}