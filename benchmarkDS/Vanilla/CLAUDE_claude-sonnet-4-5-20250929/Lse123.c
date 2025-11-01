
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char* processString(const char* input) {
    if (input == NULL) {
        return NULL;
    }
    
    int len = strlen(input);
    char* result = (char*)malloc((len + 1) * sizeof(char));
    strcpy(result, input);
    
    // Trim trailing whitespace
    int end = len - 1;
    while (end >= 0 && isspace(result[end])) {
        end--;
    }
    
    // Trim newlines
    while (end >= 0 && (result[end] == '\\n' || result[end] == '\\r')) {
        end--;
    }
    
    // Null terminate the string
    result[end + 1] = '\\0';
    
    return result;
}

int main() {
    // Test case 1: String with trailing spaces
    char* result1 = processString("hello world   ");
    printf("Test 1: '%s'\\n", result1);
    free(result1);
    
    // Test case 2: String with trailing newlines
    char* result2 = processString("hello world\\n\\n");
    printf("Test 2: '%s'\\n", result2);
    free(result2);
    
    // Test case 3: String with trailing spaces and newlines
    char* result3 = processString("hello world  \\n\\r ");
    printf("Test 3: '%s'\\n", result3);
    free(result3);
    
    // Test case 4: String with no trailing whitespace
    char* result4 = processString("hello world");
    printf("Test 4: '%s'\\n", result4);
    free(result4);
    
    // Test case 5: Empty string
    char* result5 = processString("");
    printf("Test 5: '%s'\\n", result5);
    free(result5);
    
    return 0;
}
