
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

void processArguments(int argc, char* argv[]) {
    // Validate we have at least 2 arguments
    if (argc < 2) {
        printf("Error: Insufficient arguments\\n");
        return;
    }
    
    // Convert string at index 1 to integer with validation
    char* endptr;
    errno = 0;
    long index_long = strtol(argv[1], &endptr, 10);
    
    // Check for conversion errors
    if (errno != 0 || *endptr != '\\0' || endptr == argv[1]) {
        printf("Error: Invalid index format\\n");
        return;
    }
    
    // Check if value fits in int range
    if (index_long < 0 || index_long > INT_MAX) {
        printf("Error: Index out of valid range\\n");
        return;
    }
    
    int index = (int)index_long;
    
    // Validate index is within bounds
    if (index < 0 || index >= argc) {
        printf("Error: Index out of bounds\\n");
        return;
    }
    
    // Print the argument at the specified index
    printf("%s\\n", argv[index]);
}

int main() {
    // Test case 1: Valid index
    char* test1[] = {"arg0", "2", "Hello", "World"};
    printf("Test 1: ");
    processArguments(4, test1);
    
    // Test case 2: Index out of bounds
    char* test2[] = {"arg0", "10", "Hello"};
    printf("Test 2: ");
    processArguments(3, test2);
    
    // Test case 3: Invalid index format
    char* test3[] = {"arg0", "abc", "Hello"};
    printf("Test 3: ");
    processArguments(3, test3);
    
    // Test case 4: Insufficient arguments
    char* test4[] = {"arg0"};
    printf("Test 4: ");
    processArguments(1, test4);
    
    // Test case 5: Access first argument
    char* test5[] = {"First", "0", "Second", "Third"};
    printf("Test 5: ");
    processArguments(4, test5);
    
    return 0;
}
