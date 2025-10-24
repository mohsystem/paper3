
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>

int convertToInteger(const char* input, int* result) {
    if (input == NULL || result == NULL) {
        return 0;
    }
    
    while (isspace(*input)) input++;
    
    if (*input == '\\0') {
        return 0;
    }
    
    char* endptr;
    errno = 0;
    long val = strtol(input, &endptr, 10);
    
    if (errno == ERANGE || val > INT_MAX || val < INT_MIN) {
        return 0;
    }
    
    while (isspace(*endptr)) endptr++;
    
    if (*endptr != '\\0') {
        return 0;
    }
    
    *result = (int)val;
    return 1;
}

void runTests() {
    const char* testCases[] = {"123", "-456", "0", "abc", "12.34"};
    int numTests = 5;
    
    printf("Running test cases:\\n");
    for (int i = 0; i < numTests; i++) {
        int result;
        if (convertToInteger(testCases[i], &result)) {
            printf("Input: '%s' -> Output: %d\\n", testCases[i], result);
        } else {
            printf("Input: '%s' -> Output: Invalid (cannot convert)\\n", testCases[i]);
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        int result;
        if (convertToInteger(argv[1], &result)) {
            printf("Converted integer: %d\\n", result);
        } else {
            printf("Invalid integer input: %s\\n", argv[1]);
        }
    } else {
        runTests();
    }
    
    return 0;
}
