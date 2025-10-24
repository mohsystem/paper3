
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
    int idx = 0;
    for (int i = 0; phoneNumber[i] != '\\0'; i++) {
        if (isdigit(phoneNumber[i])) {
            digitsOnly[idx++] = phoneNumber[i];
        }
    }
    digitsOnly[idx] = '\\0';
    
    // Check if it starts with country code 1 and has 11 digits
    int start = 0;
    if (idx == 11 && digitsOnly[0] == '1') {
        start = 1;
        idx = 10;
    } else if (idx != 10) {
        free(result);
        return NULL;
    }
    
    // Validate area code (first digit must be 2-9)
    if (digitsOnly[start] < '2' || digitsOnly[start] > '9') {
        free(result);
        return NULL;
    }
    
    // Validate exchange code (fourth digit must be 2-9)
    if (digitsOnly[start + 3] < '2' || digitsOnly[start + 3] > '9') {
        free(result);
        return NULL;
    }
    
    // Copy the cleaned number
    strcpy(result, &digitsOnly[start]);
    return result;
}

int main() {
    // Test case 1
    char* result1 = cleanPhoneNumber("+1 (613)-995-0253");
    printf("Test 1: %s\\n", result1 ? result1 : "NULL");
    free(result1);
    
    // Test case 2
    char* result2 = cleanPhoneNumber("613-995-0253");
    printf("Test 2: %s\\n", result2 ? result2 : "NULL");
    free(result2);
    
    // Test case 3
    char* result3 = cleanPhoneNumber("1 613 995 0253");
    printf("Test 3: %s\\n", result3 ? result3 : "NULL");
    free(result3);
    
    // Test case 4
    char* result4 = cleanPhoneNumber("613.995.0253");
    printf("Test 4: %s\\n", result4 ? result4 : "NULL");
    free(result4);
    
    // Test case 5
    char* result5 = cleanPhoneNumber("(234) 567-8901");
    printf("Test 5: %s\\n", result5 ? result5 : "NULL");
    free(result5);
    
    return 0;
}
