#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

/**
 * @brief Converts a string to Jaden Case.
 * 
 * In Jaden Case, the first letter of each word is capitalized.
 * This function allocates new memory for the result. The caller is responsible 
 * for freeing this memory using free().
 *
 * @param phrase The input null-terminated string.
 * @return A pointer to a new null-terminated string in Jaden Case, 
 *         or NULL if the input is NULL or if memory allocation fails.
 */
char* toJadenCase(const char* phrase) {
    // Rule #1: Validate input
    if (phrase == NULL) {
        return NULL;
    }

    size_t len = strlen(phrase);
    
    // Rule #3: Adhere to buffer boundaries by allocating sufficient memory.
    char* result = (char*)malloc(len + 1);
    if (result == NULL) {
        // Handle memory allocation failure
        perror("Failed to allocate memory");
        return NULL;
    }

    // Rule #2: Use safer memory copy functions. memcpy is safe when sizes are checked.
    // Copy the entire string including the null terminator.
    memcpy(result, phrase, len + 1);

    bool capitalizeNext = true;
    for (size_t i = 0; i < len; ++i) {
        // The cast to unsigned char is crucial for ctype functions to avoid
        // undefined behavior with negative char values.
        if (capitalizeNext && isalpha((unsigned char)result[i])) {
            result[i] = toupper((unsigned char)result[i]);
            capitalizeNext = false;
        } else if (isspace((unsigned char)result[i])) {
            capitalizeNext = true;
        }
    }

    return result;
}

/**
 * @brief Helper function to run a single test case.
 * 
 * This function calls toJadenCase, prints the original and resulting strings,
 * and frees the allocated memory.
 * @param test_str The string to test.
 */
void run_test_case(const char* test_str) {
    printf("Original:      \"%s\"\n", test_str ? test_str : "NULL");
    char* jadenCased = toJadenCase(test_str);
    if (jadenCased != NULL) {
        printf("Jaden-Cased:   \"%s\"\n", jadenCased);
        free(jadenCased); // Ensure allocated memory is freed exactly once.
    } else {
        printf("Jaden-Cased:   NULL\n");
    }
    printf("--------------------------------------------------------\n");
}


int main() {
    // 5 test cases
    run_test_case("How can mirrors be real if our eyes aren't real");
    run_test_case("most trees are blue");
    run_test_case("a b c d e f g");
    run_test_case("");
    run_test_case("   leading and trailing spaces   ");
    
    return 0;
}