#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>

// Function to transform a string as per the requirements.
// The caller is responsible for freeing the returned string.
char* accum(const char* s) {
    if (s == NULL) {
        return NULL;
    }

    size_t n = strlen(s);

    // According to the problem description, the input string includes only letters from a..z and A..Z.
    // However, as a good practice, we validate the input.
    for (size_t i = 0; i < n; ++i) {
        if (!isalpha((unsigned char)s[i])) {
            // Return NULL to indicate invalid input.
            return NULL;
        }
    }

    if (n == 0) {
        // Return a dynamically allocated empty string.
        char* empty_str = (char*)malloc(1);
        if (empty_str == NULL) {
            return NULL; // Allocation failure
        }
        empty_str[0] = '\0';
        return empty_str;
    }

    // Pre-calculate the total length of the resulting string.
    // The length of the i-th segment (0-indexed) is i+1.
    // Total length of all segments is sum of (i+1) for i=0 to n-1, which is n*(n+1)/2.
    // There are n-1 hyphens for n > 1.
    // We assume n is small enough not to cause overflow in size_t.
    size_t total_len = (n * (n + 1)) / 2;
    if (n > 1) {
        total_len += n - 1;
    }

    // Allocate memory for the result string plus the null terminator.
    char* result = (char*)malloc(total_len + 1);
    if (result == NULL) {
        return NULL; // Allocation failure
    }

    char* p = result; // Use a pointer to fill the new string.
    for (size_t i = 0; i < n; ++i) {
        if (i > 0) {
            *p++ = '-';
        }
        *p++ = toupper((unsigned char)s[i]);
        for (size_t j = 0; j < i; ++j) {
            *p++ = tolower((unsigned char)s[i]);
        }
    }
    *p = '\0'; // Null-terminate the string.

    return result;
}

void run_test_case(const char* input) {
    printf("accum(\"%s\") -> ", input);
    char* result = accum(input);
    if (result != NULL) {
        printf("\"%s\"\n", result);
        free(result);
    } else {
        printf("Error (e.g., invalid input or memory allocation failed)\n");
    }
}

int main() {
    // 5 test cases
    run_test_case("abcd");
    run_test_case("RqaEzty");
    run_test_case("cwAt");
    run_test_case("Z");
    run_test_case("");
    return 0;
}