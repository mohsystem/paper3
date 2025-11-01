
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char* trimString(const char* input) {
    // Check if input is NULL
    if (input == NULL) {
        return NULL;
    }
    
    // Allocate memory for the result
    size_t len = strlen(input);
    char* result = (char*)malloc(len + 1);
    if (result == NULL) {
        return NULL;
    }
    strcpy(result, input);
    
    // Trim trailing whitespace
    int endIndex = len - 1;
    while (endIndex >= 0 && isspace((unsigned char)result[endIndex])) {
        endIndex--;
    }
    
    // Trim newlines from the end
    while (endIndex >= 0 && (result[endIndex] == '\\n' || result[endIndex] == '\\r')) {
        endIndex--;
    }
    
    // Null terminate the string
    result[endIndex + 1] = '\\0';
    
    return result;
}

int main() {
    // Test cases
    const char* test1 = "hello world   ";
    const char* test2 = "test\\n\\n";
    const char* test3 = "spaces and newlines  \\n\\n  ";
    const char* test4 = "no trailing";
    const char* test5 = "   \\n\\n";
    
    char* result1 = trimString(test1);
    char* result2 = trimString(test2);
    char* result3 = trimString(test3);
    char* result4 = trimString(test4);
    char* result5 = trimString(test5);
    
    printf("Test 1: '%s'\\n", result1);
    printf("Test 2: '%s'\\n", result2);
    printf("Test 3: '%s'\\n", result3);
    printf("Test 4: '%s'\\n", result4);
    printf("Test 5: '%s'\\n", result5);
    
    free(result1);
    free(result2);
    free(result3);
    free(result4);
    free(result5);
    
    return 0;
}
