
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>

/* Predefined array for demonstration */
static const int VALUES[] = {10, 20, 30, 40, 50};
static const size_t VALUES_SIZE = sizeof(VALUES) / sizeof(VALUES[0]);

/**
 * Retrieves the value at the specified index from the array.
 * Validates input to prevent out-of-bounds access.
 * 
 * @param index The index to retrieve
 * @param result Pointer to store the result (must not be NULL)
 * @return 0 on success, -1 on error
 */
int getValueAtIndex(int index, int *result) {
    /* Input validation: Check for NULL pointer to prevent dereferencing NULL */
    if (result == NULL) {
        return -1;
    }
    
    /* Initialize output parameter */
    *result = 0;
    
    /* Input validation: Check if index is within valid bounds */
    /* This prevents buffer overflow and out-of-bounds access */
    if (index < 0 || (size_t)index >= VALUES_SIZE) {
        /* Fail closed: return error for invalid index */
        return -1;
    }
    
    /* Safe array access after bounds validation */
    *result = VALUES[index];
    return 0;
}

/**
 * Validates and parses integer input string.
 * Prevents integer overflow and validates format.
 * 
 * @param str Input string
 * @param value Pointer to store parsed value
 * @return 0 on success, -1 on error
 */
int parseInteger(const char *str, int *value) {
    char *endptr = NULL;
    long temp;
    
    /* Input validation: Check for NULL pointers */
    if (str == NULL || value == NULL) {
        return -1;
    }
    
    /* Input validation: Check for empty string or whitespace only */
    if (*str == '\\0' || (*str == '\\n' && *(str + 1) == '\\0')) {
        return -1;
    }
    
    /* Reset errno to detect overflow */
    errno = 0;
    
    /* Convert string to long with base 10 */
    temp = strtol(str, &endptr, 10);
    
    /* Input validation: Check for conversion errors */
    /* Check for overflow/underflow */
    if (errno == ERANGE || temp > INT_MAX || temp < INT_MIN) {
        return -1;
    }
    
    /* Check if entire string was consumed (no trailing garbage) */
    if (endptr == str || (*endptr != '\\0' && *endptr != '\\n')) {
        return -1;
    }
    
    *value = (int)temp;
    return 0;
}

int main(void) {
    int result;
    char buffer[32]; /* Buffer for user input - limited size to prevent excessive memory usage */
    int userIndex;
    
    /* Initialize buffer to zero for safety */
    memset(buffer, 0, sizeof(buffer));
    
    /* Test cases */
    printf("Running test cases:\\n");
    
    if (getValueAtIndex(0, &result) == 0) {
        printf("Test 1 (index=0): %d\\n", result);
    }
    if (getValueAtIndex(2, &result) == 0) {
        printf("Test 2 (index=2): %d\\n", result);
    }
    if (getValueAtIndex(4, &result) == 0) {
        printf("Test 3 (index=4): %d\\n", result);
    }
    if (getValueAtIndex(-1, &result) != 0) {
        printf("Test 4 (index=-1): Invalid\\n");
    }
    if (getValueAtIndex(10, &result) != 0) {
        printf("Test 5 (index=10): Invalid\\n");
    }
    
    /* Interactive mode */
    printf("\\nEnter an index (0-%zu): ", VALUES_SIZE - 1);
    fflush(stdout); /* Ensure prompt is displayed */
    
    /* Input validation: Use fgets for safe input reading with bounds checking */
    /* fgets is safer than gets and scanf as it limits the number of characters read */
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        /* Generic error message - do not leak internal details */
        fprintf(stderr, "Error: Failed to read input.\\n");
        return EXIT_FAILURE;
    }
    
    /* Input validation: Parse and validate integer input */
    if (parseInteger(buffer, &userIndex) != 0) {
        /* Generic error message - do not leak internal details */
        fprintf(stderr, "Error: Invalid input format. Expected integer.\\n");
        return EXIT_FAILURE;
    }
    
    /* Retrieve value at index with validation */
    if (getValueAtIndex(userIndex, &result) == 0) {
        printf("Value at index %d: %d\\n", userIndex, result);
    } else {
        /* Generic error message - do not leak internal details */
        fprintf(stderr, "Error: Index out of valid range.\\n");
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
