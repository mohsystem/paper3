
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

char* cleanPhoneNumber(const char* phoneNumber) {
    if (phoneNumber == NULL) {
        return NULL;
    }
    
    // Allocate memory for result
    char* result = (char*)malloc(11 * sizeof(char));
    if (result == NULL) {
        return NULL;
    }
    
    // Remove all non-digit characters
    char digitsOnly[20];
    int digitCount = 0;
    
    for (int i = 0; phoneNumber[i] != '\\0'; i++) {
        if (isdigit(phoneNumber[i])) {
            digitsOnly[digitCount++] = phoneNumber[i];
        }
    }
    digitsOnly[digitCount] = '\\0';
    
    // Check if it starts with country code 1 and has 11 digits
    int startIndex = 0;
    if (digitCount == 11 && digitsOnly[0] == '1') {
        startIndex = 1;
        digitCount--;
    }
    
    // Validate the cleaned number
    if (digitCount != 10) {
        free(result);
        return NULL;
    }
    
    // Validate area code (first digit must be 2-9)
    if (digitsOnly[startIndex] < '2' || digitsOnly[startIndex] > '9') {
        free(result);
        return NULL;
    }
    
    // Validate exchange code (first digit must be 2-9)
    if (digitsOnly[startIndex + 3] < '2' || digitsOnly[startIndex + 3] > '9') {
        free(result);
        return NULL;
    }
    
    // Copy the cleaned number to result
    strncpy(result, digitsOnly + startIndex, 10);
    result[10] = '\\0';
    
    return result;
}

int main() {
    // Test cases
    char* result1 = cleanPhoneNumber("+1 (613)-995-0253");
    printf("Test 1: %s\\n", result1 ? result1 : "NULL");
    free(result1);
    
    char* result2 = cleanPhoneNumber("613-995-0253");
    printf("Test 2: %s\\n", result2 ? result2 : "NULL");
    free(result2);
    
    char* result3 = cleanPhoneNumber("1 613 995 0253");
    printf("Test 3: %s\\n", result3 ? result3 : "NULL");
    free(result3);
    
    char* result4 = cleanPhoneNumber("613.995.0253");
    printf("Test 4: %s\\n", result4 ? result4 : "NULL");
    free(result4);
    
    char* result5 = cleanPhoneNumber("(234) 567-8901");
    printf("Test 5: %s\\n", result5 ? result5 : "NULL");
    free(result5);
    
    return 0;
}
