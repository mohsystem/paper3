#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/**
 * @brief Cleans up user-entered phone numbers.
 * Removes punctuation and the country code (1) if present.
 * @param phoneNumber The raw phone number string.
 * @return A dynamically allocated 10-digit phone number string,
 *         or an empty string for invalid NANP numbers.
 *         The caller is responsible for freeing the returned string.
 */
char* cleanPhoneNumber(const char* phoneNumber) {
    int len = strlen(phoneNumber);
    char* digits = (char*)malloc(len + 1);
    if (digits == NULL) return NULL;

    int j = 0;
    for (int i = 0; i < len; i++) {
        if (isdigit((unsigned char)phoneNumber[i])) {
            digits[j++] = phoneNumber[i];
        }
    }
    digits[j] = '\0';

    int digits_len = strlen(digits);
    char* result = NULL;

    if (digits_len == 11 && digits[0] == '1') {
        result = (char*)malloc(11); // 10 digits + null terminator
        if (result != NULL) {
            strcpy(result, digits + 1);
        }
    } else if (digits_len == 10) {
        result = (char*)malloc(11); // 10 digits + null terminator
        if (result != NULL) {
            strcpy(result, digits);
        }
    }

    free(digits);

    // If result is still NULL, it means the number was invalid.
    // Allocate an empty string to return.
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
        "(223) 456-7890"
    };
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < numTestCases; i++) {
        char* cleanedNumber = cleanPhoneNumber(testCases[i]);
        if (cleanedNumber != NULL) {
            printf("Input: \"%s\", Output: \"%s\"\n", testCases[i], cleanedNumber);
            free(cleanedNumber);
        } else {
             printf("Input: \"%s\", Output: Memory allocation failed\n", testCases[i]);
        }
    }

    return 0;
}