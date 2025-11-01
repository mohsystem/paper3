
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

int processValue(const char* input) {
    char* endptr;
    long long value;
    
    // Validate input is not NULL or empty
    if (input == NULL || strlen(input) == 0) {
        fprintf(stderr, "Error: Input cannot be NULL or empty\\n");
        return -1;
    }
    
    // Reset errno before conversion
    errno = 0;
    value = strtoll(input, &endptr, 10);
    
    // Check for conversion errors
    if (errno == ERANGE) {
        fprintf(stderr, "Error: Number out of range\\n");
        return -1;
    }
    
    // Check if entire string was converted
    if (*endptr != '\\0' && *endptr != '\\n') {
        fprintf(stderr, "Error: Invalid number format\\n");
        return -1;
    }
    
    // Check for overflow when adding 1000
    if (value > LLONG_MAX - 1000) {
        fprintf(stderr, "Error: Integer overflow detected\\n");
        return -1;
    }
    
    if (value < LLONG_MIN + 1000) {
        fprintf(stderr, "Error: Integer underflow detected\\n");
        return -1;
    }
    
    long long result = value + 1000;
    printf("%lld\\n", result);
    return 0;
}

int main(int argc, char* argv[]) {
    // Test cases
    const char* testCases[] = {"100", "0", "-500", "999999999", "abc"};
    
    printf("Running test cases:\\n");
    for (int i = 0; i < 5; i++) {
        printf("\\nTest case %d: input = %s\\n", i + 1, testCases[i]);
        processValue(testCases[i]);
    }
    
    // Process command line argument if provided
    if (argc > 1) {
        printf("\\nProcessing command line argument:\\n");
        return processValue(argv[1]);
    }
    
    return 0;
}
