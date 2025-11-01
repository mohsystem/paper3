
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <errno.h>
#include <string.h>

/* Predefined array with fixed bounds */
static const int id_sequence[] = {100, 200, 300, 400, 500};
static const size_t id_sequence_length = sizeof(id_sequence) / sizeof(id_sequence[0]);

/**
 * Retrieves value at specified index from id_sequence array.
 * Validates index to prevent out-of-bounds access.
 * 
 * @param index The index to retrieve from id_sequence
 * @param result Pointer to store the result value (must not be NULL)
 * @return true if successful, false if invalid index
 */
bool getValueAtIndex(int index, int* result) {
    /* Input validation: Check null pointer */
    if (result == NULL) {
        fprintf(stderr, "Error: Invalid result pointer\\n");
        return false;
    }
    
    /* Initialize output to safe value */
    *result = 0;
    
    /* Input validation: Check bounds to prevent array access violations */
    /* Reject negative indices and indices beyond array length */
    /* Cast to size_t only after validating non-negative */
    if (index < 0 || (size_t)index >= id_sequence_length) {
        /* Fail closed: return error without exposing internal details */
        fprintf(stderr, "Error: Invalid index\\n");
        return false;
    }
    
    /* Bounds checked: safe to access array */
    *result = id_sequence[index];
    return true;
}

/**
 * Prompts user for index and returns corresponding value.
 * Validates all user input before processing.
 * 
 * @param result Pointer to store the result value (must not be NULL)
 * @return true if successful, false on error
 */
bool promptAndGetValue(int* result) {
    char buffer[32]; /* Fixed size buffer for input */
    char* endptr = NULL;
    long temp_value;
    int index;
    
    /* Input validation: Check null pointer */
    if (result == NULL) {
        fprintf(stderr, "Error: Invalid result pointer\\n");
        return false;
    }
    
    /* Initialize output to safe value */
    *result = 0;
    
    /* Prompt user with clear range information */
    printf("Enter index (0-%zu): ", id_sequence_length - 1);
    
    /* Input validation: Use fgets for safe input (prevents buffer overflow) */
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        fprintf(stderr, "Error: Input read failed\\n");
        return false;
    }
    
    /* Remove newline if present */
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\\n') {
        buffer[len - 1] = '\\0';
    }
    
    /* Input validation: Convert string to integer with error checking */
    errno = 0;
    temp_value = strtol(buffer, &endptr, 10);
    
    /* Check for conversion errors */
    if (endptr == buffer || *endptr != '\\0') {
        fprintf(stderr, "Error: Invalid input type\\n");
        return false;
    }
    
    /* Check for overflow/underflow */
    if (errno == ERANGE || temp_value < INT_MIN || temp_value > INT_MAX) {
        fprintf(stderr, "Error: Value out of range\\n");
        return false;
    }
    
    /* Safe cast after validation */
    index = (int)temp_value;
    
    /* Delegate to validated function */
    return getValueAtIndex(index, result);
}

int main(void) {
    int value;
    
    /* Test case 1: Valid index at start */
    if (getValueAtIndex(0, &value)) {
        printf("Test 1 - Index 0: %d\\n", value);
    }
    
    /* Test case 2: Valid index in middle */
    if (getValueAtIndex(2, &value)) {
        printf("Test 2 - Index 2: %d\\n", value);
    }
    
    /* Test case 3: Valid index at end */
    if (getValueAtIndex(4, &value)) {
        printf("Test 3 - Index 4: %d\\n", value);
    }
    
    /* Test case 4: Invalid negative index */
    if (!getValueAtIndex(-1, &value)) {
        printf("Test 4 - Index -1: Failed as expected\\n");
    }
    
    /* Test case 5: Invalid index beyond bounds */
    if (!getValueAtIndex(10, &value)) {
        printf("Test 5 - Index 10: Failed as expected\\n");
    }
    
    return 0;
}
