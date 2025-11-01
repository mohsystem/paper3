
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#define OFFSET 1000

int addOffset(const char* input, int* result) {
    if (input == NULL || result == NULL) {
        return -1;
    }
    
    char* endptr;
    errno = 0;
    long value = strtol(input, &endptr, 10);
    
    // Check for conversion errors
    if (endptr == input || *endptr != '\\0') {
        fprintf(stderr, "Invalid input: not a valid integer\\n");
        return -1;
    }
    
    if (errno == ERANGE || value > INT_MAX || value < INT_MIN) {
        fprintf(stderr, "Integer overflow detected\\n");
        return -1;
    }
    
    // Check for overflow when adding offset
    if (value > INT_MAX - OFFSET) {
        fprintf(stderr, "Integer overflow detected\\n");
        return -1;
    }
    
    *result = (int)value + OFFSET;
    return 0;
}

int main() {
    // Test cases
    const char* testCases[] = {"100", "0", "-500", "2147482647", "50"};
    int numTests = 5;
    
    for (int i = 0; i < numTests; i++) {
        int result;
        printf("Input: %s -> ", testCases[i]);
        if (addOffset(testCases[i], &result) == 0) {
            printf("Output: %d\\n", result);
        } else {
            printf("Error occurred\\n");
        }
    }
    
    return 0;
}
