#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * Cleans up a user-entered phone number string.
 *
 * @param phoneNumber The raw phone number string.
 * @return A dynamically allocated 10-digit NANP number string, 
 *         or NULL if the input is invalid. The caller is responsible for freeing the returned memory.
 */
char* cleanPhoneNumber(const char* phoneNumber) {
    if (phoneNumber == NULL) {
        return NULL;
    }

    // 1. Filter out non-digit characters.
    size_t len = strlen(phoneNumber);
    char* cleaned = (char*)malloc(len + 1);
    if (cleaned == NULL) {
        return NULL; // Memory allocation failed
    }

    int digit_count = 0;
    for (size_t i = 0; i < len; ++i) {
        if (isdigit((unsigned char)phoneNumber[i])) {
            cleaned[digit_count++] = phoneNumber[i];
        }
    }
    cleaned[digit_count] = '\0';

    char* result = NULL;

    // 2. Handle country code if present.
    if (digit_count == 11 && cleaned[0] == '1') {
        result = (char*)malloc(11); // 10 digits + null terminator
        if (result != NULL) {
            strcpy(result, cleaned + 1);
        }
    } 
    // 3. Check if the result is a 10-digit number.
    else if (digit_count == 10) {
        result = (char*)malloc(11);
        if (result != NULL) {
            strcpy(result, cleaned);
        }
    }

    free(cleaned); // Free the temporary buffer
    
    // 4. If not, result remains NULL, indicating an invalid number.
    return result;
}

int main() {
    const char* testCases[] = {
        "+1 (613)-995-0253",
        "613-995-0253",
        "1 613 995 0253",
        "613.995.0253",
        "12345" // Invalid case
    };

    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < numTestCases; ++i) {
        char* result = cleanPhoneNumber(testCases[i]);
        printf("Input: \"%s\", Cleaned: \"%s\"\n", testCases[i], result ? result : "(invalid)");
        if (result != NULL) {
            free(result); // The caller must free the memory
        }
    }

    return 0;
}