#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>

/**
 * Validates if the given string is a valid IPv4 address.
 *
 * @param ip_str The string to validate.
 * @return true if the string is a valid IPv4 address, false otherwise.
 */
bool isValidIP(const char* ip_str) {
    if (ip_str == NULL) {
        return false;
    }

    size_t len = strlen(ip_str);
    // Basic length check for efficiency. Min: "0.0.0.0" (7), Max: "255.255.255.255" (15)
    if (len < 7 || len > 15) {
        return false;
    }

    char part_str[4]; // Max part is 3 digits + null terminator
    int part_len = 0;
    int num_parts = 0;
    int num_dots = 0;
    
    for (size_t i = 0; i < len; ++i) {
        char c = ip_str[i];

        if (c == '.') {
            num_dots++;
            // A part cannot be empty. e.g., "1..2.3.4" or ".1.2.3"
            if (part_len == 0) {
                return false;
            }
            
            part_str[part_len] = '\0';
            
            // Validate the extracted part
            // A part cannot have leading zeros unless it is "0" itself.
            if (part_len > 1 && part_str[0] == '0') {
                return false;
            }
            // The number must be in the range [0, 255].
            // isdigit check below ensures atoi won't get non-numeric input.
            int num = atoi(part_str);
            if (num < 0 || num > 255) { 
                return false;
            }

            num_parts++;
            part_len = 0; // Reset for the next part
        } else if (isdigit((unsigned char)c)) {
            // Part cannot be longer than 3 digits. This also prevents buffer overflow.
            if (part_len >= 3) {
                return false;
            }
            part_str[part_len++] = c;
        } else {
            // Invalid character found
            return false;
        }
    }
    
    // Process the last part after the loop
    // Trailing dot check, e.g., "1.2.3."
    if (part_len == 0) {
        return false;
    }
    
    part_str[part_len] = '\0';
    if (part_len > 1 && part_str[0] == '0') {
        return false;
    }
    int num = atoi(part_str);
    if (num < 0 || num > 255) {
        return false;
    }
    num_parts++;

    // A valid IPv4 address must have exactly 4 parts and 3 dots.
    return num_parts == 4 && num_dots == 3;
}

int main() {
    const char* testCases[] = {
        "127.0.0.1",       // Valid
        "256.0.0.0",       // Invalid - range
        "192.168.01.1",    // Invalid - leading zero
        "1.2.3",           // Invalid - format/parts count
        "a.b.c.d",         // Invalid - non-numeric
        NULL
    };

    for (int i = 0; testCases[i] != NULL; i++) {
        printf("IP: \"%s\" is %s\n", testCases[i], isValidIP(testCases[i]) ? "Valid" : "Invalid");
    }

    return 0;
}