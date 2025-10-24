
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* reverseString(const char* input) {
    if (input == NULL) {
        return NULL;
    }
    
    int len = strlen(input);
    char* reversed = (char*)malloc((len + 1) * sizeof(char));
    
    if (reversed == NULL) {
        return NULL;
    }
    
    for (int i = 0; i < len; i++) {
        reversed[i] = input[len - 1 - i];
    }
    reversed[len] = '\\0';
    
    return reversed;
}

int main() {
    char* result;
    
    // Test case 1: Normal string
    result = reverseString("hello");
    printf("Test 1: %s\\n", result);
    free(result);
    
    // Test case 2: String with spaces
    result = reverseString("hello world");
    printf("Test 2: %s\\n", result);
    free(result);
    
    // Test case 3: Empty string
    result = reverseString("");
    printf("Test 3: %s\\n", result);
    free(result);
    
    // Test case 4: Single character
    result = reverseString("a");
    printf("Test 4: %s\\n", result);
    free(result);
    
    // Test case 5: String with special characters
    result = reverseString("Hello123!@#");
    printf("Test 5: %s\\n", result);
    free(result);
    
    return 0;
}
