
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

/**
 * Retrieves and prints the command line argument at the specified index.
 * 
 * @param argv The command line arguments array
 * @param argc The number of command line arguments
 * @param index The index to retrieve (0-based)
 * @return 0 on success, non-zero on error
 */
int getArgumentAtIndex(char* argv[], int argc, int index) {
    // Validate argv is not NULL to prevent null pointer dereference
    if (argv == NULL) {
        printf("Error: Arguments array is NULL\\n");
        return 1;
    }
    
    // Validate index is non-negative to prevent negative array access
    if (index < 0) {
        printf("Error: Index cannot be negative\\n");
        return 1;
    }
    
    // Check if index is within bounds to prevent buffer over-read
    if (index >= argc) {
        printf("Error: Index out of bounds\\n");
        return 1;
    }
    
    // Validate the specific argument pointer is not NULL
    if (argv[index] == NULL) {
        printf("Error: Argument at index is NULL\\n");
        return 1;
    }
    
    // Print the argument at the valid index
    printf("%s\\n", argv[index]);
    return 0;
}

int main(int argc, char* argv[]) {
    // Test case 1: Valid index within bounds
    char* test1[] = {"arg0", "arg1", "arg2", "arg3"};
    printf("Test 1 - Index 1: ");
    getArgumentAtIndex(test1, 4, 1);
    
    // Test case 2: Valid index at boundary
    char* test2[] = {"first", "second", "third"};
    printf("Test 2 - Index 2: ");
    getArgumentAtIndex(test2, 3, 2);
    
    // Test case 3: Index out of bounds (should return error)
    char* test3[] = {"only", "two"};
    printf("Test 3 - Index 5: ");
    getArgumentAtIndex(test3, 2, 5);
    
    // Test case 4: Negative index (should return error)
    char* test4[] = {"some", "args"};
    printf("Test 4 - Index -1: ");
    getArgumentAtIndex(test4, 2, -1);
    
    // Test case 5: Empty array with index 0 (should return error)
    printf("Test 5 - Index 0 on empty: ");
    getArgumentAtIndex(NULL, 0, 0);
    
    // Actual program logic: process command line arguments if provided
    // Validate argc and argv before processing
    if (argc > 1 && argv != NULL && argv[1] != NULL) {
        // Parse the first argument as the index with validation
        char* endptr = NULL;
        errno = 0;  // Reset errno before conversion
        long long_index = strtol(argv[1], &endptr, 10);
        
        // Validate conversion was successful
        if (errno == ERANGE || endptr == argv[1] || *endptr != '\\0') {
            printf("Error: Invalid index format\\n");
            return 1;
        }
        
        // Check for overflow when converting long to int
        if (long_index > INT_MAX || long_index < INT_MIN) {
            printf("Error: Index value out of range\\n");
            return 1;
        }
        
        int index = (int)long_index;
        
        // The actual arguments start from index 2 (after program name and index)
        // Calculate the new argc for the subset of arguments
        int actual_argc = argc - 2;
        
        // Retrieve and print the argument at the specified index
        // Pass &argv[2] as the new base and adjusted argc
        if (actual_argc > 0) {
            getArgumentAtIndex(&argv[2], actual_argc, index);
        } else {
            printf("Error: Index out of bounds\\n");
        }
    }
    
    return 0;
}
