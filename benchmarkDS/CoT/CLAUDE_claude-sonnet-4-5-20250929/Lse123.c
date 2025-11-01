
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

void processString(int argc, char* argv[], char* result, size_t maxLen) {
    // Check if exactly one argument is provided
    if (argc != 2) {
        strncpy(result, "Error: Exactly one argument required", maxLen - 1);
        result[maxLen - 1] = '\\0';
        return;
    }
    
    // Check if the argument is valid
    if (argv[1] == NULL) {
        result[0] = '\\0';
        return;
    }
    
    // Safely copy input to result buffer
    size_t len = strlen(argv[1]);
    if (len >= maxLen) {
        len = maxLen - 1;
    }
    strncpy(result, argv[1], len);
    result[len] = '\\0';
    
    // Trim trailing whitespace
    int end = (int)len - 1;
    while (end >= 0 && isspace((unsigned char)result[end])) {
        end--;
    }
    
    // Trim newlines
    while (end >= 0 && (result[end] == '\\n' || result[end] == '\\r')) {
        end--;
    }
    
    // Null terminate the string
    result[end + 1] = '\\0';
}

int main() {
    const size_t bufSize = 256;
    char result[bufSize];
    
    // Test case 1: Normal string with trailing spaces
    char* test1[] = {"program", "Hello World   "};
    processString(2, test1, result, bufSize);
    printf("Test 1: '%s'\\n", result);
    
    // Test case 2: String with trailing newlines
    char* test2[] = {"program", "Hello\\n\\n"};
    processString(2, test2, result, bufSize);
    printf("Test 2: '%s'\\n", result);
    
    // Test case 3: String with both spaces and newlines
    char* test3[] = {"program", "Test String  \\n  \\n"};
    processString(2, test3, result, bufSize);
    printf("Test 3: '%s'\\n", result);
    
    // Test case 4: Empty string
    char* test4[] = {"program", ""};
    processString(2, test4, result, bufSize);
    printf("Test 4: '%s'\\n", result);
    
    // Test case 5: No trailing whitespace
    char* test5[] = {"program", "Clean"};
    processString(2, test5, result, bufSize);
    printf("Test 5: '%s'\\n", result);
    
    return 0;
}
