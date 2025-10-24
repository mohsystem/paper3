#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/**
 * @brief Converts a positive integer into its expanded form string.
 *
 * This function takes a positive integer and returns a dynamically allocated string
 * that represents the number as a sum of its parts, each multiplied by its place value.
 * For example, 70304 becomes "70000 + 300 + 4".
 * The caller is responsible for freeing the returned string.
 * The function uses secure C practices like snprintf to prevent buffer overflows.
 *
 * @param n The positive integer to convert. Must be greater than 0.
 * @return A dynamically allocated C-string representing the expanded form, or NULL on error.
 */
char* expandedForm(long long n) {
    if (n <= 0) {
        // As per prompt, n > 0. Handle defensively.
        char* empty_str = (char*)malloc(1);
        if (empty_str != NULL) {
            empty_str[0] = '\0';
        }
        return empty_str;
    }

    char num_str[25]; // Sufficient for a 64-bit long long
    int len = snprintf(num_str, sizeof(num_str), "%lld", n);

    if (len < 0 || (size_t)len >= sizeof(num_str)) {
        return NULL; // snprintf error
    }

    // Estimate max result size. A 20-digit number can result in a string
    // of roughly 270 chars. 512 is a safe buffer size.
    size_t buffer_size = 512;
    char* result = (char*)malloc(buffer_size);
    if (result == NULL) {
        return NULL; // Memory allocation failed
    }
    result[0] = '\0';

    char* current_ptr = result;
    size_t remaining_size = buffer_size;
    bool first_term = true;

    for (int i = 0; i < len; ++i) {
        if (num_str[i] != '0') {
            int written;
            if (!first_term) {
                written = snprintf(current_ptr, remaining_size, " + ");
                if (written < 0 || (size_t)written >= remaining_size) {
                    free(result);
                    return NULL; // Buffer overflow would have occurred
                }
                current_ptr += written;
                remaining_size -= written;
            }

            written = snprintf(current_ptr, remaining_size, "%c", num_str[i]);
            if (written < 0 || (size_t)written >= remaining_size) {
                free(result);
                return NULL;
            }
            current_ptr += written;
            remaining_size -= written;
            
            for (int j = 0; j < len - 1 - i; ++j) {
                written = snprintf(current_ptr, remaining_size, "0");
                if (written < 0 || (size_t)written >= remaining_size) {
                    free(result);
                    return NULL;
                }
                current_ptr += written;
                remaining_size -= written;
            }
            first_term = false;
        }
    }
    
    return result;
}

int main() {
    // Test cases
    long long test_cases[] = {12, 42, 70304, 999, 8};
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);

    for (int i = 0; i < num_tests; ++i) {
        long long test_num = test_cases[i];
        char* result = expandedForm(test_num);
        
        printf("Input: %lld\n", test_num);
        if (result != NULL) {
            printf("Output: \"%s\"\n", result);
            free(result); // Free the dynamically allocated memory
        } else {
            fprintf(stderr, "Error processing %lld\n", test_num);
        }
        printf("--------------------\n");
    }

    return 0;
}