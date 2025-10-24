#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

/**
 * @brief Validates if the given string is a valid IPv4 address.
 *
 * A valid IPv4 address consists of four octets separated by dots.
 * Each octet must be a number between 0 and 255.
 * Leading zeros are not allowed, except for the number "0" itself.
 * This implementation safely parses the string without modification.
 *
 * @param ip The null-terminated string to validate.
 * @return true if the string is a valid IPv4 address, false otherwise.
 */
bool isValidIP(const char* ip) {
    if (ip == NULL) {
        return false;
    }

    int num;
    int parts;
    int digits;
    const char *ptr = ip;

    // A valid IP address must have exactly 4 parts.
    for (parts = 0; parts < 4; ++parts) {
        digits = 0;
        num = 0;
        
        // A leading zero is only allowed if the part is exactly "0".
        if (*ptr == '0' && isdigit((unsigned char)*(ptr + 1))) {
            return false;
        }

        // Parse all digits of the current part.
        while (isdigit((unsigned char)*ptr)) {
            num = num * 10 + (*ptr - '0');
            // The value of an octet cannot exceed 255.
            if (num > 255) {
                return false;
            }
            ptr++;
            digits++;
        }

        // Each part must have at least one digit (e.g., handles "1..2.3.4").
        if (digits == 0) {
            return false;
        }

        // If this is not the last part, a dot must follow.
        if (parts < 3) {
            if (*ptr != '.') {
                return false;
            }
            ptr++; // Move past the dot for the next iteration.
        }
    }

    // After successfully parsing 4 parts, the string must end.
    // This catches trailing characters (e.g., "1.2.3.4.5" or "1.2.3.4a").
    if (*ptr != '\0') {
        return false;
    }

    return true;
}

int main() {
    const char* testCases[] = {
        "192.168.1.1",      // Valid
        "255.255.255.255",  // Valid
        "0.0.0.0",          // Valid
        "1.2.3.4",          // Valid
        "192.168.1.256",    // Invalid: octet > 255
        "192.168.01.1",     // Invalid: leading zero
        "192.168.1",        // Invalid: too few parts
        "a.b.c.d",          // Invalid: non-numeric parts
        "1.2.3.4.",         // Invalid: trailing dot
        ".1.2.3.4",         // Invalid: leading dot
        "1..2.3.4"          // Invalid: consecutive dots
    };
    int num_tests = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < num_tests; ++i) {
        printf("IP: %s is %s\n", testCases[i], isValidIP(testCases[i]) ? "valid" : "invalid");
    }

    return 0;
}