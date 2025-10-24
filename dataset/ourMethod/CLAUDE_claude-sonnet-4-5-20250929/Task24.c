
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

// Validates ATM PIN: must be exactly 4 or 6 digits, nothing else
// Security: Input validation prevents injection attacks and ensures type safety
bool validatePin(const char* pin) {
    // Check for NULL pointer - fail closed
    if (pin == NULL) {
        return false;
    }
    
    // Get length using strlen (safe for null-terminated strings)
    size_t len = strlen(pin);
    
    // Validate length is exactly 4 or 6
    if (len != 4 && len != 6) {
        return false;
    }
    
    // Validate each character is a digit [0-9]
    // This prevents any non-digit characters including letters, symbols, whitespace
    // Using unsigned char cast to avoid undefined behavior with negative char values
    for (size_t i = 0; i < len; i++) {
        if (!isdigit((unsigned char)pin[i])) {
            return false;
        }
    }
    
    return true;
}

// Test cases
int main() {
    // Test case 1: Valid 4-digit PIN
    printf("Test 1 - '1234': %s\\n", validatePin("1234") ? "true" : "false");
    
    // Test case 2: Invalid 5-digit PIN
    printf("Test 2 - '12345': %s\\n", validatePin("12345") ? "true" : "false");
    
    // Test case 3: Invalid PIN with letter
    printf("Test 3 - 'a234': %s\\n", validatePin("a234") ? "true" : "false");
    
    // Test case 4: Valid 6-digit PIN
    printf("Test 4 - '123456': %s\\n", validatePin("123456") ? "true" : "false");
    
    // Test case 5: Empty string
    printf("Test 5 - '': %s\\n", validatePin("") ? "true" : "false");
    
    return 0;
}
