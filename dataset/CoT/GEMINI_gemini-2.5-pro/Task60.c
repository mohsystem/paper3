#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/**
 * Converts a number to its expanded form string.
 * For example: 70304 becomes "70000 + 300 + 4"
 * @param num The positive integer to convert (using long long for wider range).
 * @return A dynamically allocated string with the expanded form. The caller must free this string.
 */
char* expandedForm(long long num) {
    char* parts[20]; // A long long can have at most ~19 digits. 20 is safe.
    int count = 0;
    long long place = 1;
    long long temp_num = num;

    while (temp_num > 0) {
        long long rem = temp_num % 10;
        if (rem != 0) {
            long long part_val = rem * place;
            // Calculate length needed for the number string + null terminator
            int len = snprintf(NULL, 0, "%lld", part_val);
            parts[count] = (char*)malloc(len + 1);
            if (parts[count] == NULL) {
                // Handle malloc failure: free previously allocated parts
                for (int i = 0; i < count; i++) {
                    free(parts[i]);
                }
                return NULL; 
            }
            sprintf(parts[count], "%lld", part_val);
            count++;
        }
        temp_num /= 10;
        place *= 10;
    }
    
    // According to the prompt, num > 0, so count will be at least 1.
    if (count == 0) {
        char* result = (char*)malloc(2);
        if(result == NULL) return NULL;
        strcpy(result, "0");
        return result;
    }

    // Calculate total length for the final string
    size_t total_len = 0;
    for (int i = 0; i < count; i++) {
        total_len += strlen(parts[i]);
    }
    if (count > 1) {
        total_len += (count - 1) * 3; // For " + " separators
    }
    total_len += 1; // For null terminator

    char* result = (char*)malloc(total_len);
    if (result == NULL) {
        // Handle malloc failure
        for (int i = 0; i < count; i++) {
            free(parts[i]);
        }
        return NULL;
    }
    result[0] = '\0'; // Initialize as empty string for strcat

    // Concatenate parts in reverse order of how they were found
    for (int i = count - 1; i >= 0; i--) {
        strcat(result, parts[i]);
        if (i > 0) {
            strcat(result, " + ");
        }
    }

    // Free the memory for individual part strings
    for (int i = 0; i < count; i++) {
        free(parts[i]);
    }

    return result;
}

void run_test_case(long long num) {
    char* result = expandedForm(num);
    if (result) {
        printf("Input: %lld, Output: %s\n", num, result);
        free(result); // Free the memory allocated by the function
    } else {
        printf("Failed to generate expanded form for %lld (memory allocation error).\n", num);
    }
}

int main() {
    long long test_cases[] = {12, 42, 70304, 9000000, 806};
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    for(int i = 0; i < num_tests; i++) {
        run_test_case(test_cases[i]);
    }
    return 0;
}