
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>

#define OFFSET 1000
/* Prevent integer overflow by setting safe bounds */
#define MAX_INPUT_VALUE (LLONG_MAX - OFFSET)
#define MIN_INPUT_VALUE (LLONG_MIN - OFFSET)

/* Trims leading and trailing whitespace from a string in-place */
static void trim_whitespace(char* str) {
    if (str == NULL) return;
    
    /* Trim leading whitespace */
    char* start = str;
    while (isspace((unsigned char)*start)) start++;
    
    /* Move trimmed string to beginning if needed */
    if (start != str) {
        size_t len = strlen(start);
        memmove(str, start, len + 1);
    }
    
    /* Trim trailing whitespace */
    size_t len = strlen(str);
    while (len > 0 && isspace((unsigned char)str[len - 1])) {
        str[--len] = '\\0';
    }
}

/* Adds an offset of 1000 to the input value */
int addOffset(const char* input, long long* result) {
    /* Input validation: check for NULL pointer */
    if (input == NULL || result == NULL) {
        fprintf(stderr, "Invalid arguments\\n");
        return -1;
    }
    
    /* Allocate memory for sanitized input - bounded allocation */
    size_t input_len = strlen(input);
    if (input_len > 1024) {  /* Prevent excessive allocation */
        fprintf(stderr, "Input too long\\n");
        return -1;
    }
    
    char* sanitized = (char*)malloc(input_len + 1);
    if (sanitized == NULL) {
        fprintf(stderr, "Memory allocation failed\\n");
        return -1;
    }
    
    /* Copy and sanitize input */
    strncpy(sanitized, input, input_len);
    sanitized[input_len] = '\\0';  /* Ensure null termination */
    trim_whitespace(sanitized);
    
    /* Check for empty string after trimming */
    if (sanitized[0] == '\\0') {
        fprintf(stderr, "Input cannot be empty\\n");
        free(sanitized);
        return -1;
    }
    
    /* Parse input with error checking */
    char* end = NULL;
    errno = 0;
    long long value = strtoll(sanitized, &end, 10);
    
    /* Free sanitized buffer before returning */
    free(sanitized);
    sanitized = NULL;
    
    /* Check for conversion errors */
    if (errno == ERANGE) {
        fprintf(stderr, "Input value out of range\\n");
        return -1;
    }
    if (end == sanitized || *end != '\\0') {
        fprintf(stderr, "Invalid numeric input\\n");
        return -1;
    }
    
    /* Integer overflow/underflow check before addition */
    if (value > MAX_INPUT_VALUE) {
        fprintf(stderr, "Input value too large, would cause overflow\\n");
        return -1;
    }
    if (value < MIN_INPUT_VALUE) {
        fprintf(stderr, "Input value too small, would cause underflow\\n");
        return -1;
    }
    
    /* Safe addition after validation */
    *result = value + OFFSET;
    return 0;
}

int main(void) {
    long long result = 0;
    
    /* Test case 1: Normal positive value */
    printf("Test 1:\\n");
    if (addOffset("500", &result) == 0) {
        printf("Input: 500, Result: %lld\\n", result);
    }
    
    /* Test case 2: Negative value */
    printf("\\nTest 2:\\n");
    if (addOffset("-200", &result) == 0) {
        printf("Input: -200, Result: %lld\\n", result);
    }
    
    /* Test case 3: Zero */
    printf("\\nTest 3:\\n");
    if (addOffset("0", &result) == 0) {
        printf("Input: 0, Result: %lld\\n", result);
    }
    
    /* Test case 4: Large value near max (should prevent overflow) */
    printf("\\nTest 4:\\n");
    if (addOffset("9223372036854774807", &result) == 0) {
        printf("Input: 9223372036854774807, Result: %lld\\n", result);
    }
    
    /* Test case 5: Invalid input */
    printf("\\nTest 5:\\n");
    if (addOffset("invalid", &result) == 0) {
        printf("Input: invalid, Result: %lld\\n", result);
    }
    
    return 0;
}
