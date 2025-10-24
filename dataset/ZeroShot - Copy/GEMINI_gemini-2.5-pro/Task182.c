#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/**
 * Cleans a phone number by removing punctuation and country code.
 * Validates the number against NANP rules.
 * 
 * Note: The caller is responsible for freeing the memory of the returned string.
 * 
 * @param input The raw phone number string.
 * @return A heap-allocated 10-digit string on success, or NULL on error.
 */
char* cleanPhoneNumber(const char* input) {
    if (input == NULL) {
        return NULL;
    }

    size_t inputLen = strlen(input);
    char* digits = (char*)malloc(inputLen + 1);
    if (digits == NULL) {
        return NULL; // Memory allocation failed
    }

    int j = 0;
    for (size_t i = 0; i < inputLen; i++) {
        if (isdigit((unsigned char)input[i])) {
            digits[j++] = input[i];
        }
    }
    digits[j] = '\0';

    char* final_digits = digits;
    size_t digitsLen = strlen(final_digits);

    if (digitsLen == 11) {
        if (final_digits[0] == '1') {
            final_digits++; // Move pointer forward one position
            digitsLen--;
        } else {
            free(digits);
            return NULL;
        }
    }

    if (digitsLen != 10) {
        free(digits);
        return NULL;
    }

    if (final_digits[0] == '0' || final_digits[0] == '1') {
        free(digits);
        return NULL;
    }
    if (final_digits[3] == '0' || final_digits[3] == '1') {
        free(digits);
        return NULL;
    }

    char* result = (char*)malloc(11); // 10 digits + 1 for null terminator
    if (result == NULL) {
        free(digits);
        return NULL;
    }
    
    strncpy(result, final_digits, 10);
    result[10] = '\0';
    
    free(digits);
    return result;
}

int main() {
    const char* testCases[] = {
        "+1 (613)-995-0253",
        "613-995-0253",
        "1 613 995 0253",
        "613.995.0253",
        "(223) 456-7890",
        // Invalid cases for testing robustness
        "1 (123) 456-7890", // Area code starts with 1
        "(223) 056-7890", // Exchange code starts with 0
        "2 (223) 456-7890", // Invalid 11-digit number
        "(223) 456-789",    // Too short
        "123456789012"      // Too long
    };
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < numTestCases; i++) {
        const char* testCase = testCases[i];
        printf("Input: \"%s\" -> ", testCase);
        
        char* cleaned = cleanPhoneNumber(testCase);
        if (cleaned != NULL) {
            printf("Output: %s\n", cleaned);
            free(cleaned); // Free the allocated memory
        } else {
            printf("Error: Invalid number format\n");
        }
    }

    return 0;
}