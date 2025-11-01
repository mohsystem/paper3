
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>

// Function to get value from array with bounds checking
// Returns the value at the given index if valid, otherwise returns -1
int getValueFromArray(const int* array, int size, int index) {
    // Security: Validate array is not NULL
    if (array == NULL) {
        fprintf(stderr, "Error: Array is NULL\\n");
        return -1;
    }
    
    // Security: Validate size is non-negative
    if (size < 0) {
        fprintf(stderr, "Error: Invalid size\\n");
        return -1;
    }
    
    // Security: Bounds checking to prevent out-of-bounds access
    if (index < 0 || index >= size) {
        fprintf(stderr, "Error: Index out of bounds\\n");
        return -1;
    }
    
    // Safe to access array element after validation
    return array[index];
}

int main(void) {
    // Predefined array for testing
    const int id_sequence[] = {10, 20, 30, 40, 50};
    const int size = sizeof(id_sequence) / sizeof(id_sequence[0]);
    
    printf("Enter index: ");
    
    // Security: Use fgets for safe input reading with size limit
    char input_buffer[32];
    if (fgets(input_buffer, sizeof(input_buffer), stdin) == NULL) {
        fprintf(stderr, "Error: Failed to read input\\n");
        return 1;
    }
    
    // Security: Validate and parse input as integer
    char* endptr = NULL;
    errno = 0;
    long temp_index = strtol(input_buffer, &endptr, 10);
    
    // Security: Check for conversion errors
    if (errno == ERANGE || endptr == input_buffer || (*endptr != '\\n' && *endptr != '\\0')) {
        fprintf(stderr, "Error: Invalid integer input\\n");
        return 1;
    }
    
    // Security: Check if value fits in int range
    if (temp_index < INT_MIN || temp_index > INT_MAX) {
        fprintf(stderr, "Error: Index out of integer range\\n");
        return 1;
    }
    
    int index = (int)temp_index;
    
    // Call function and get result
    int result = getValueFromArray(id_sequence, size, index);
    
    // Return the result as exit code (values outside 0-255 may be truncated)
    return (result >= 0 && result <= 255) ? result : 1;
}

// Test cases (comment out main and uncomment this to run tests)
/*
void runTests(void) {
    const int id_sequence[] = {10, 20, 30, 40, 50};
    const int size = sizeof(id_sequence) / sizeof(id_sequence[0]);
    
    printf("Test 1 - Valid index 0: %d\\n", getValueFromArray(id_sequence, size, 0)); // Expected: 10
    printf("Test 2 - Valid index 4: %d\\n", getValueFromArray(id_sequence, size, 4)); // Expected: 50
    printf("Test 3 - Invalid index -1: %d\\n", getValueFromArray(id_sequence, size, -1)); // Expected: -1
    printf("Test 4 - Invalid index 5: %d\\n", getValueFromArray(id_sequence, size, 5)); // Expected: -1
    printf("Test 5 - NULL array: %d\\n", getValueFromArray(NULL, 0, 0)); // Expected: -1
}
*/
