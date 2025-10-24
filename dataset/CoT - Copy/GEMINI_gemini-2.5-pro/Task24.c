#include <stdio.h>
#include <string.h>
#include <stdbool.h> // For bool type
#include <ctype.h>   // For isdigit

/**
 * @brief Validates an ATM PIN.
 * A valid PIN must be exactly 4 or 6 digits long and contain only digits.
 * 
 * @param pin A constant character pointer to the string to validate.
 * @return true if the PIN is valid, false otherwise.
 */
bool validatePin(const char *pin) {
    if (pin == NULL) {
        return false;
    }

    // Get the length of the string
    size_t len = strlen(pin);

    // Check if the length is not 4 or 6
    if (len != 4 && len != 6) {
        return false;
    }

    // Iterate through the string to check if all characters are digits
    for (size_t i = 0; i < len; i++) {
        // isdigit expects an int; casting to unsigned char is a safe way
        // to handle all possible character values before they are promoted to int.
        if (!isdigit((unsigned char)pin[i])) {
            return false;
        }
    }

    // If all checks pass, the PIN is valid
    return true;
}

int main() {
    printf("Testing C version:\n");
    // Test Case 1: Valid 4-digit PIN
    printf("\"1234\"   -->  %s\n", validatePin("1234") ? "true" : "false");
    // Test Case 2: Invalid length
    printf("\"12345\"  -->  %s\n", validatePin("12345") ? "true" : "false");
    // Test Case 3: Invalid character
    printf("\"a234\"   -->  %s\n", validatePin("a234") ? "true" : "false");
    // Test Case 4: Valid 6-digit PIN
    printf("\"451352\" -->  %s\n", validatePin("451352") ? "true" : "false");
    // Test Case 5: Invalid characters and length
    printf("\"-1234\"  -->  %s\n", validatePin("-1234") ? "true" : "false");

    return 0;
}