
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <string.h>

/* Returns the value at the given index from the array
   Security: Validates all parameters to prevent buffer overflow/underflow */
int getValueFromArray(const int* array, int size, int index) {
    /* Validate array pointer is not NULL to prevent null pointer dereference */
    if (array == NULL) {
        fprintf(stderr, "Error: Null array provided\\n");
        return -1;
    }
    
    /* Validate size is positive to prevent negative indexing and integer issues */
    if (size <= 0) {
        fprintf(stderr, "Error: Invalid size\\n");
        return -1;
    }
    
    /* Bounds checking: ensure index is within valid range [0, size)
       This prevents both buffer underflow (negative index) and overflow (index >= size) */
    if (index < 0 || index >= size) {
        fprintf(stderr, "Error: Index out of bounds\\n");
        return -1;
    }
    
    /* Safe to access array at this point - all validations passed */
    return array[index];
}

int main(void) {
    /* Predefined test array - fixed size for security, prevents unbounded allocation */
    const int testArray[] = {10, 20, 30, 40, 50};
    const int size = sizeof(testArray) / sizeof(testArray[0]);
    
    char buffer[32];  /* Fixed size buffer to prevent overflow */
    int index = 0;
    int result = 0;
    char* endptr = NULL;
    long parsed_value = 0;
    
    /* Initialize buffer to prevent uninitialized memory access */
    memset(buffer, 0, sizeof(buffer));
    
    /* Read input from stdin with size limit - prevents buffer overflow */
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        fprintf(stderr, "Error: Failed to read input\\n");
        return 1;
    }
    
    /* Remove newline if present */
    buffer[strcspn(buffer, "\\n")] = '\\0';
    
    /* Reset errno before strtol for proper error detection */
    errno = 0;
    endptr = NULL;
    
    /* Parse input as integer - treat as untrusted */
    parsed_value = strtol(buffer, &endptr, 10);
    
    /* Validate parsing: check for errors and ensure entire string was consumed */
    if (errno != 0 || endptr == buffer || *endptr != '\\0') {
        fprintf(stderr, "Error: Invalid input format\\n");
        return 1;
    }
    
    /* Check for integer overflow/underflow during conversion */
    if (parsed_value > INT_MAX || parsed_value < INT_MIN) {
        fprintf(stderr, "Error: Value out of range\\n");
        return 1;
    }
    
    index = (int)parsed_value;
    
    /* Call function with validated parameters */
    result = getValueFromArray(testArray, size, index);
    
    /* Return result as exit code (constrained to valid range 0-255)
       Use absolute value and modulo to ensure valid exit code */
    return abs(result) % 256;
}

/*
Test cases (compile and run with different inputs):
echo "2" | ./program  -> Expected: exit code 30
echo "0" | ./program  -> Expected: exit code 10
echo "4" | ./program  -> Expected: exit code 50
echo "-1" | ./program -> Expected: exit code 1 (error)
echo "10" | ./program -> Expected: exit code 1 (error)
*/
