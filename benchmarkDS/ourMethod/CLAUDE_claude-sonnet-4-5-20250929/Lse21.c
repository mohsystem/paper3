
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>

/* Process input value by adding 1000 and printing result */
/* Returns 0 on success, -1 on error */
int processValue(const char* input) {
    /* Input validation: check if input is NULL or empty */
    if (input == NULL || input[0] == '\\0') {
        fprintf(stderr, "Error: Input cannot be NULL or empty\\n");
        return -1;
    }
    
    /* Validate input format before parsing */
    const char* ptr = input;
    while (isspace((unsigned char)*ptr)) ptr++; /* Skip leading whitespace */
    
    if (*ptr == '-' || *ptr == '+') ptr++; /* Skip sign */
    
    int hasDigit = 0;
    while (*ptr != '\\0') {
        if (isdigit((unsigned char)*ptr)) {
            hasDigit = 1;
            ptr++;
        } else if (isspace((unsigned char)*ptr)) {
            /* Allow trailing whitespace */
            while (isspace((unsigned char)*ptr)) ptr++;
            break;
        } else {
            fprintf(stderr, "Error: Invalid number format\\n");
            return -1;
        }
    }
    
    if (!hasDigit) {
        fprintf(stderr, "Error: Invalid number format\\n");
        return -1;
    }
    
    /* Parse input as long with validation */
    char* endptr = NULL;
    errno = 0;
    long value = strtol(input, &endptr, 10);
    
    /* Check for parsing errors */
    if (errno == ERANGE || (errno != 0 && value == 0)) {
        fprintf(stderr, "Error: Number out of range\\n");
        return -1;
    }
    
    if (endptr == input) {
        fprintf(stderr, "Error: Invalid number format\\n");
        return -1;
    }
    
    /* Check for integer overflow before addition */
    if (value > LONG_MAX - 1000) {
        fprintf(stderr, "Error: Value would overflow\\n");
        return -1;
    }
    
    if (value < LONG_MIN + 1000) {
        fprintf(stderr, "Error: Value would underflow\\n");
        return -1;
    }
    
    /* Safe addition after overflow check */
    long result = value + 1000;
    
    /* Print result to console using safe output method */
    printf("%ld\\n", result);
    
    return 0;
}

int main(void) {
    /* Test cases with input validation */
    printf("Test 1: Valid positive number\\n");
    processValue("500");
    
    printf("\\nTest 2: Valid negative number\\n");
    processValue("-200");
    
    printf("\\nTest 3: Zero\\n");
    processValue("0");
    
    printf("\\nTest 4: Large number\\n");
    processValue("999999");
    
    printf("\\nTest 5: Invalid input\\n");
    processValue("not_a_number");
    
    return 0;
}
