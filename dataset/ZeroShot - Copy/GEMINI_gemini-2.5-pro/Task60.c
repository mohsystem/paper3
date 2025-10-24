#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * Converts a positive number into its expanded form string.
 *
 * @param num The number to convert, must be greater than 0.
 * @return A dynamically allocated string with the expanded form.
 *         The caller is responsible for freeing this memory.
 */
char* expandedForm(long long num) {
    // According to the problem description, num will be > 0.
    // Adding a check for robustness.
    if (num <= 0) {
        char* empty = (char*)malloc(1);
        if (empty == NULL) return NULL; // Allocation failed
        empty[0] = '\0';
        return empty;
    }
    
    char num_str[25]; // A 64-bit long long has at most 20 digits. 25 is safe.
    int len = sprintf(num_str, "%lld", num);
    
    // Allocate a buffer. A safe over-estimation is 5 times the number of digits.
    size_t buffer_size = len * 5;
    char* result = (char*)malloc(buffer_size);
    if (result == NULL) {
        return NULL; // Allocation failed
    }
    
    char* current_pos = result;
    int first_part = 1;

    for (int i = 0; i < len; ++i) {
        if (num_str[i] != '0') {
            if (!first_part) {
                // Safely append the separator
                current_pos += sprintf(current_pos, " + ");
            }
            
            // Append the digit
            *current_pos++ = num_str[i];

            // Append the zeros
            for (int j = 0; j < len - 1 - i; ++j) {
                *current_pos++ = '0';
            }
            
            first_part = 0;
        }
    }
    *current_pos = '\0'; // Null-terminate the string
    
    return result;
}

int main() {
    long long test_cases[] = {12, 42, 70304, 9000000, 1};
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);

    for (int i = 0; i < num_tests; ++i) {
        char* result = expandedForm(test_cases[i]);
        if (result != NULL) {
            printf("%lld -> %s\n", test_cases[i], result);
            free(result); // Free the dynamically allocated memory
        } else {
            fprintf(stderr, "Memory allocation failed for test case %lld\n", test_cases[i]);
        }
    }

    return 0;
}