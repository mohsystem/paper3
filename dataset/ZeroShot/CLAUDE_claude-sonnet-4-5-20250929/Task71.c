
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>

int* convertStringToInteger(const char* input) {
    if (input == NULL || strlen(input) == 0) {
        printf("Error: Invalid input - empty or null string\\n");
        return NULL;
    }
    
    char* endptr;
    errno = 0;
    
    long value = strtol(input, &endptr, 10);
    
    // Skip trailing whitespace
    while (*endptr != '\\0' && isspace(*endptr)) {
        endptr++;
    }
    
    // Check for conversion errors
    if (errno == ERANGE || value > INT_MAX || value < INT_MIN) {
        printf("Error: Number out of range - '%s'\\n", input);
        return NULL;
    }
    
    if (endptr == input || *endptr != '\\0') {
        printf("Error: Invalid input - cannot convert '%s' to integer\\n", input);
        return NULL;
    }
    
    int* result = (int*)malloc(sizeof(int));
    if (result == NULL) {
        printf("Error: Memory allocation failed\\n");
        return NULL;
    }
    
    *result = (int)value;
    return result;
}

int main(int argc, char* argv[]) {
    // Test cases
    const char* testCases[] = {"123", "-456", "0", "abc", "12.34"};
    
    printf("Running test cases:\\n");
    for (int i = 0; i < 5; i++) {
        printf("Input: '%s' -> ", testCases[i]);
        int* result = convertStringToInteger(testCases[i]);
        if (result != NULL) {
            printf("Output: %d\\n", *result);
            free(result);
        }
    }
    
    // Command line argument processing
    if (argc > 1) {
        printf("\\nCommand line argument:\\n");
        printf("Input: '%s' -> ", argv[1]);
        int* result = convertStringToInteger(argv[1]);
        if (result != NULL) {
            printf("Output: %d\\n", *result);
            free(result);
        }
    } else {
        printf("\\nNo command line arguments provided.\\n");
    }
    
    return 0;
}
