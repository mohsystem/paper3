
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

char* cleanPhoneNumber(const char* phoneNumber) {
    if (phoneNumber == NULL) {
        return NULL;
    }
    
    // Allocate memory for result (max 11 digits + null terminator)
    char* digitsOnly = (char*)malloc(12 * sizeof(char));
    if (digitsOnly == NULL) {
        return NULL;
    }
    
    int digitIndex = 0;
    
    // Extract only digits
    for (int i = 0; phoneNumber[i] != '\\0'; i++) {
        if (isdigit(phoneNumber[i])) {
            digitsOnly[digitIndex++] = phoneNumber[i];
        }
    }
    digitsOnly[digitIndex] = '\\0';
    
    // Check if empty after cleaning
    if (digitIndex == 0) {
        free(digitsOnly);
        return NULL;
    }
    
    // Handle country code 1
    if (digitIndex == 11 && digitsOnly[0] == '1') {
        memmove(digitsOnly, digitsOnly + 1, 10);
        digitsOnly[10] = '\\0';
        digitIndex = 10;
    }
    
    // Validate length
    if (digitIndex != 10) {
        free(digitsOnly);
        return NULL;
    }
    
    // Validate NANP format: NXX NXX-XXXX
    // First digit of area code (N) must be 2-9
    if (digitsOnly[0] < '2' || digitsOnly[0] > '9') {
        free(digitsOnly);
        return NULL;
    }
    
    // Fourth digit (first digit of exchange code) must be 2-9
    if (digitsOnly[3] < '2' || digitsOnly[3] > '9') {
        free(digitsOnly);
        return NULL;
    }
    
    return digitsOnly;
}

int main() {
    char* result;
    
    // Test case 1: Phone number with country code and special characters
    result = cleanPhoneNumber("+1 (613)-995-0253");
    printf("Test 1: %s\\n", result ? result : "NULL");
    free(result);
    
    // Test case 2: Phone number with dashes
    result = cleanPhoneNumber("613-995-0253");
    printf("Test 2: %s\\n", result ? result : "NULL");
    free(result);
    
    // Test case 3: Phone number with country code and spaces
    result = cleanPhoneNumber("1 613 995 0253");
    printf("Test 3: %s\\n", result ? result : "NULL");
    free(result);
    
    // Test case 4: Phone number with dots
    result = cleanPhoneNumber("613.995.0253");
    printf("Test 4: %s\\n", result ? result : "NULL");
    free(result);
    
    // Test case 5: Phone number with parentheses
    result = cleanPhoneNumber("(234) 567-8901");
    printf("Test 5: %s\\n", result ? result : "NULL");
    free(result);
    
    return 0;
}
