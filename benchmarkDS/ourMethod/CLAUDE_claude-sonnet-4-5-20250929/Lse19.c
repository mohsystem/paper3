
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>

/**
 * Adds 1000 to the provided integer value.
 * 
 * @param value The input integer value
 * @param result Pointer to store the result
 * @return 0 on success, -1 on overflow
 */
int addThousand(int value, int* result) {
    /* Validate output pointer */
    if (result == NULL) {
        return -1;
    }
    
    /* Check for integer overflow before performing addition */
    if (value > INT_MAX - 1000) {
        fprintf(stderr, "Error: Integer overflow would occur\\n");
        return -1;
    }
    /* Check for integer underflow */
    if (value < INT_MIN + 1000) {
        fprintf(stderr, "Error: Integer underflow would occur\\n");
        return -1;
    }
    
    *result = value + 1000;
    return 0;
}

/**
 * Safely parses a string to integer with validation.
 * 
 * @param str The input string
 * @param value Pointer to store the parsed value
 * @return 0 on success, -1 on error
 */
int safeParseInt(const char* str, int* value) {
    char* endptr = NULL;
    long result;
    
    /* Validate inputs */
    if (str == NULL || value == NULL) {
        return -1;
    }
    
    /* Validate input length to prevent excessive processing */
    size_t len = strlen(str);
    if (len == 0 || len > 15) {
        fprintf(stderr, "Error: Invalid input length\\n");
        return -1;
    }
    
    /* Reset errno before conversion */
    errno = 0;
    result = strtol(str, &endptr, 10);
    
    /* Check for conversion errors */
    if (errno == ERANGE || result > INT_MAX || result < INT_MIN) {
        fprintf(stderr, "Error: Value out of integer range\\n");
        return -1;
    }
    
    /* Check if entire string was converted (skip trailing whitespace) */
    if (endptr == str) {
        fprintf(stderr, "Error: Invalid input format\\n");
        return -1;
    }
    
    /* Skip trailing whitespace */
    while (*endptr != '\\0' && isspace((unsigned char)*endptr)) {
        endptr++;
    }
    
    if (*endptr != '\\0') {
        fprintf(stderr, "Error: Invalid input format\\n");
        return -1;
    }
    
    *value = (int)result;
    return 0;
}

int main(int argc, char* argv[]) {
    int testCases[] = {0, 100, -500, 2147482647, -2147482648};
    int result = 0;
    int i;
    
    /* Test cases */
    for (i = 0; i < 5; i++) {
        if (addThousand(testCases[i], &result) == 0) {
            printf("Input: %d, Result: %d\\n", testCases[i], result);
        } else {
            fprintf(stderr, "Error processing value %d\\n", testCases[i]);
        }
    }
    
    /* If command line argument provided, process it */
    if (argc > 1) {
        int inputValue = 0;
        
        /* Validate and parse input */
        if (safeParseInt(argv[1], &inputValue) != 0) {
            fprintf(stderr, "Error: Failed to parse command line argument\\n");
            return 1;
        }
        
        if (addThousand(inputValue, &result) == 0) {
            printf("Command line input: %d, Result: %d\\n", inputValue, result);
        } else {
            fprintf(stderr, "Error: Operation failed\\n");
            return 1;
        }
    }
    
    return 0;
}
