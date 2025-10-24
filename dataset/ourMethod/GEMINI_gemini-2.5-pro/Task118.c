#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

/**
 * @brief Validates if a given string is a valid IPv4 address.
 *
 * An IPv4 address is valid if it consists of four octets separated by dots,
 * where each octet is a decimal number from 0 to 255.
 * Leading zeros are not allowed (e.g., "01" is invalid), except for the number "0" itself.
 * This function parses the string character by character to avoid unsafe functions
 * like strtok or sscanf.
 *
 * @param ip The C-string to validate.
 * @return true if the string is a valid IPv4 address, false otherwise.
 */
bool isValidIPv4(const char* ip) {
    // Rule #1: Ensure all input is validated.
    if (ip == NULL) {
        return false;
    }

    int octet_val = 0;
    int num_digits = 0;
    int num_parts = 0;
    const char *p = ip;

    while (*p) {
        if (isdigit((unsigned char)*p)) {
            num_digits++;
            // An octet cannot have more than 3 digits.
            if (num_digits > 3) return false;
            // Check for leading zero (e.g., "01"). A single "0" is valid.
            if (num_digits > 1 && octet_val == 0) return false;

            octet_val = octet_val * 10 + (*p - '0');
            // Check if octet value exceeds 255.
            if (octet_val > 255) {
                return false;
            }
        } else if (*p == '.') {
            // A dot must be preceded by at least one digit (no empty octets).
            if (num_digits == 0) return false;

            num_parts++;
            // Reset for the next octet.
            octet_val = 0;
            num_digits = 0;
        } else {
            // Invalid character.
            return false;
        }
        p++;
    }

    // After the loop, check the last part.
    // Must not end with a dot (e.g., "1.2.3."), which means last octet was empty.
    if (num_digits == 0) return false;

    num_parts++;
    
    // An IPv4 address must have exactly 4 parts.
    return num_parts == 4;
}

int main() {
    const char* testCases[] = {
        "192.168.1.1",       // Valid
        "256.0.0.1",         // Invalid range
        "192.168.01.1",      // Invalid leading zero
        "192.168.1.a",       // Invalid character
        "1.2.3"              // Invalid structure (too few parts)
    };
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < numTestCases; ++i) {
        printf("Is \"%s\" a valid IPv4 address? %s\n", testCases[i], isValidIPv4(testCases[i]) ? "true" : "false");
    }

    printf("\n--- Additional Test Cases ---\n");
    const char* moreTestCases[] = {
        "0.0.0.0",           // Valid
        "255.255.255.255",   // Valid
        "1.2.3.4.5",         // Invalid structure (too many parts)
        "1..2.3.4",          // Invalid structure (empty octet)
        "1.2.3.",            // Invalid structure (trailing dot)
        "",                  // Invalid empty string
        NULL,                // Invalid null
        "123.45.67.89"       // Valid
    };
    int numMoreTestCases = sizeof(moreTestCases) / sizeof(moreTestCases[0]);

    for (int i = 0; i < numMoreTestCases; ++i) {
        const char* ip_str = moreTestCases[i] ? moreTestCases[i] : "NULL";
        printf("Is \"%s\" a valid IPv4 address? %s\n", ip_str, isValidIPv4(moreTestCases[i]) ? "true" : "false");
    }

    return 0;
}