
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

bool validatePin(const char* pin) {
    // Null check
    if (pin == NULL) {
        return false;
    }
    
    // Length validation - must be exactly 4 or 6
    size_t length = strlen(pin);
    if (length != 4 && length != 6) {
        return false;
    }
    
    // Validate each character is a digit
    for (size_t i = 0; i < length; i++) {
        if (!isdigit((unsigned char)pin[i])) {
            return false;
        }
    }
    
    return true;
}

int main() {
    // Test cases
    printf("Test 1 - '1234': %s\\n", validatePin("1234") ? "true" : "false");      // true
    printf("Test 2 - '12345': %s\\n", validatePin("12345") ? "true" : "false");    // false
    printf("Test 3 - 'a234': %s\\n", validatePin("a234") ? "true" : "false");      // false
    printf("Test 4 - '123456': %s\\n", validatePin("123456") ? "true" : "false");  // true
    printf("Test 5 - '': %s\\n", validatePin("") ? "true" : "false");              // false
    
    return 0;
}
