
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

// Function to get value from array with bounds checking
// Returns the value at the given index if valid, otherwise returns -1
int getValueFromArray(const int* array, int size, int index) {
    // Validate input parameters to prevent out-of-bounds access
    if (array == NULL) {
        fprintf(stderr, "Error: Array is NULL\\n");
        return -1;
    }
    
    // Ensure size is non-negative to prevent unsigned comparison issues
    if (size < 0) {
        fprintf(stderr, "Error: Invalid size\\n");
        return -1;
    }
    
    // Ensure index is within valid bounds [0, size-1]
    if (index < 0 || index >= size) {
        fprintf(stderr, "Error: Index out of bounds\\n");
        return -1;
    }
    
    // Safe to access array at this point
    return array[index];
}

int main(void) {
    // Predefined array for testing
    const int id_sequence[] = {10, 20, 30, 40, 50};
    const int size = sizeof(id_sequence) / sizeof(id_sequence[0]);
    
    int index;
    char buffer[32];
    
    // Read input from standard input with validation
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        fprintf(stderr, "Error: Failed to read input\\n");
        return 1;
    }
    
    // Parse input as integer with error handling
    char* endptr = NULL;
    errno = 0;
    long parsed_value = strtol(buffer, &endptr, 10);
    
    // Validate conversion was successful and complete
    if (errno != 0 || endptr == buffer || (*endptr != '\\n' && *endptr != '\\0')) {
        fprintf(stderr, "Error: Invalid integer input\\n");
        return 1;
    }
    
    // Ensure parsed value is within int range
    if (parsed_value < INT_MIN || parsed_value > INT_MAX) {
        fprintf(stderr, "Error: Value out of int range\\n");
        return 1;
    }
    
    index = (int)parsed_value;
    
    // Get value from array with bounds checking
    int result = getValueFromArray(id_sequence, size, index);
    
    // Exit with the retrieved value (or error code -1)
    // Ensure exit code is in valid range [0, 255]
    int exit_code = (result >= 0 && result <= 255) ? result : (result & 0xFF);
    return exit_code;
    
    // Test cases (uncomment to run)
    /*
    printf("Test 1 (index 0): %d\\n", getValueFromArray(id_sequence, size, 0));  // Expected: 10
    printf("Test 2 (index 2): %d\\n", getValueFromArray(id_sequence, size, 2));  // Expected: 30
    printf("Test 3 (index 4): %d\\n", getValueFromArray(id_sequence, size, 4));  // Expected: 50
    printf("Test 4 (index -1): %d\\n", getValueFromArray(id_sequence, size, -1));  // Expected: -1
    printf("Test 5 (index 10): %d\\n", getValueFromArray(id_sequence, size, 10));  // Expected: -1
    return 0;
    */
}
