
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>

/* Predefined array of id_sequence values */
static const int ID_SEQUENCE[] = {100, 200, 300, 400, 500, 600, 700, 800, 900, 1000};
static const size_t ID_SEQUENCE_LENGTH = sizeof(ID_SEQUENCE) / sizeof(ID_SEQUENCE[0]);

/**
 * Retrieves value at specified index from id_sequence array
 * @param index The index to retrieve (must be within array bounds)
 * @param result Pointer to store the result value
 * @return 0 on success, -1 on error
 */
int getValueAtIndex(int index, int *result) {
    /* Validate result pointer is not NULL */
    if (result == NULL) {
        fprintf(stderr, "Error: NULL pointer provided\\n");
        return -1;
    }
    
    /* Input validation: check if index is negative */
    if (index < 0) {
        fprintf(stderr, "Error: Invalid index: must be non-negative\\n");
        return -1;
    }
    
    /* Input validation: check if index is within bounds */
    if ((size_t)index >= ID_SEQUENCE_LENGTH) {
        fprintf(stderr, "Error: Invalid index: must be between 0 and %zu\\n", 
                ID_SEQUENCE_LENGTH - 1);
        return -1;
    }
    
    /* Return value at valid index */
    *result = ID_SEQUENCE[index];
    return 0;
}

/**
 * Prompts user for index and returns corresponding value
 * @param result Pointer to store the result value
 * @return 0 on success, -1 on error
 */
int promptAndGetValue(int *result) {
    char buffer[256];
    long index_long;
    int index;
    char *endptr = NULL;
    
    /* Validate result pointer is not NULL */
    if (result == NULL) {
        fprintf(stderr, "Error: NULL pointer provided\\n");
        return -1;
    }
    
    /* Initialize buffer to prevent use of uninitialized memory */
    memset(buffer, 0, sizeof(buffer));
    
    printf("Enter an index value (0-%zu): ", ID_SEQUENCE_LENGTH - 1);
    
    /* Use fgets for safe input reading with bounds checking */
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        fprintf(stderr, "Error: Failed to read input\\n");
        return -1;
    }
    
    /* Remove newline if present */
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\\n') {
        buffer[len - 1] = '\\0';
    }
    
    /* Reset errno before conversion */
    errno = 0;
    
    /* Convert string to long with error checking */
    index_long = strtol(buffer, &endptr, 10);
    
    /* Validate conversion was successful */
    if (errno == ERANGE || endptr == buffer || *endptr != '\\0') {
        fprintf(stderr, "Error: Invalid input type\\n");
        return -1;
    }
    
    /* Check for integer overflow when converting long to int */
    if (index_long < INT_MIN || index_long > INT_MAX) {
        fprintf(stderr, "Error: Index out of integer range\\n");
        return -1;
    }
    
    index = (int)index_long;
    
    /* Call getValueAtIndex with validated input */
    return getValueAtIndex(index, result);
}

int main(void) {
    int result = 0;
    int status = 0;
    
    /* Test case 1: Valid index at start */
    printf("\\nTest case 1: Valid index 0\\n");
    status = getValueAtIndex(0, &result);
    if (status == 0) {
        printf("Value at index 0: %d\\n", result);
    } else {
        fprintf(stderr, "Test 1 failed\\n");
    }
    
    /* Test case 2: Valid index in middle */
    printf("\\nTest case 2: Valid index 5\\n");
    status = getValueAtIndex(5, &result);
    if (status == 0) {
        printf("Value at index 5: %d\\n", result);
    } else {
        fprintf(stderr, "Test 2 failed\\n");
    }
    
    /* Test case 3: Valid index at end */
    printf("\\nTest case 3: Valid index 9\\n");
    status = getValueAtIndex(9, &result);
    if (status == 0) {
        printf("Value at index 9: %d\\n", result);
    } else {
        fprintf(stderr, "Test 3 failed\\n");
    }
    
    /* Test case 4: Invalid negative index */
    printf("\\nTest case 4: Invalid index -1\\n");
    status = getValueAtIndex(-1, &result);
    if (status != 0) {
        fprintf(stderr, "Test 4 failed (expected)\\n");
    }
    
    /* Test case 5: Invalid out of bounds index */
    printf("\\nTest case 5: Invalid index 15\\n");
    status = getValueAtIndex(15, &result);
    if (status != 0) {
        fprintf(stderr, "Test 5 failed (expected)\\n");
    }
    
    return 0;
}
