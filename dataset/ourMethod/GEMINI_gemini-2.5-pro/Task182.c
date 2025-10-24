#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/**
 * @brief Cleans up a user-entered phone number string by removing punctuation
 * and the NANP country code (1) if present.
 *
 * @param phoneNumber The raw phone number string.
 * @return A dynamically allocated 10-digit phone number string if the input is
 *         a valid NANP number, otherwise an empty string. The caller must free
 *         the returned string. Returns NULL on allocation failure.
 */
char* cleanPhoneNumber(const char* phoneNumber) {
    if (phoneNumber == NULL) {
        char* empty_result = (char*)malloc(1);
        if (empty_result) empty_result[0] = '\0';
        return empty_result;
    }
    
    // A buffer large enough for any reasonable phone number format
    char digits[50];
    int j = 0;
    size_t input_len = strlen(phoneNumber);

    for (size_t i = 0; i < input_len && j < sizeof(digits) - 1; ++i) {
        if (isdigit((unsigned char)phoneNumber[i])) {
            digits[j++] = phoneNumber[i];
        }
    }
    digits[j] = '\0';

    size_t len = strlen(digits);
    char* result = NULL;

    if (len == 11 && digits[0] == '1') {
        result = (char*)malloc(11); // 10 digits + null terminator
        if (result != NULL) {
            strncpy(result, digits + 1, 10);
            result[10] = '\0';
        }
    } else if (len == 10) {
        result = (char*)malloc(11); // 10 digits + null terminator
        if (result != NULL) {
            strncpy(result, digits, 10);
            result[10] = '\0';
        }
    }

    // If result is still NULL (invalid format or malloc failure), return an empty string.
    if (result == NULL) {
        result = (char*)malloc(1);
        if (result != NULL) {
            result[0] = '\0';
        }
    }
    
    return result;
}

int main() {
    const char* testCases[] = {
        "+1 (613)-995-0253",
        "613-995-0253",
        "1 613 995 0253",
        "613.995.0253",
        "2 (123) 456-7890" // Invalid country code
    };
    int num_tests = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < num_tests; ++i) {
        char* cleanedNumber = cleanPhoneNumber(testCases[i]);
        if (cleanedNumber != NULL) {
            printf("Original: \"%s\", Cleaned: \"%s\"\n", testCases[i], cleanedNumber);
            free(cleanedNumber);
        } else {
            printf("Original: \"%s\", Cleaned: (memory allocation failed)\n", testCases[i]);
        }
    }

    return 0;
}