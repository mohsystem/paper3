
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>

int addThousand(const char* input, int* result) {
    if (input == NULL || strlen(input) == 0) {
        return -1; // Invalid input
    }
    
    // Trim whitespace
    while (isspace(*input)) input++;
    
    if (*input == '\\0') {
        return -1; // Empty after trimming
    }
    
    char* endptr;
    errno = 0;
    long long value = strtoll(input, &endptr, 10);
    
    // Check for conversion errors
    if (errno == ERANGE || *endptr != '\\0') {
        return -1; // Invalid format or overflow
    }
    
    long long temp_result = value + 1000;
    
    // Check for overflow
    if (temp_result > INT_MAX || temp_result < INT_MIN) {
        return -2; // Overflow
    }
    
    *result = (int)temp_result;
    return 0; // Success
}

int main(int argc, char* argv[]) {
    // Test cases
    const char* testCases[] = {"100", "500", "-200", "0", "2147482647"};
    int numTests = 5;
    
    for (int i = 0; i < numTests; i++) {
        int result;
        int status = addThousand(testCases[i], &result);
        
        if (status == 0) {
            printf("Input: %s -> Output: %d\\n", testCases[i], result);
        } else if (status == -1) {
            printf("Input: %s -> Error: Invalid number format\\n", testCases[i]);
        } else if (status == -2) {
            printf("Input: %s -> Error: Result exceeds integer bounds\\n", testCases[i]);
        }
    }
    
    // Command line argument processing
    if (argc > 1) {
        int result;
        int status = addThousand(argv[1], &result);
        
        if (status == 0) {
            printf("\\nCommand line input: %s -> Output: %d\\n", argv[1], result);
        } else if (status == -1) {
            fprintf(stderr, "Error: Invalid number format\\n");
        } else if (status == -2) {
            fprintf(stderr, "Error: Result exceeds integer bounds\\n");
        }
    }
    
    return 0;
}
