
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

const char* getArgumentAtIndex(int argc, char* argv[]) {
    // Security check: Validate arguments
    if (argv == NULL || argc < 2) {
        return "Error: Insufficient arguments. Need at least 2 arguments.";
    }
    
    // Security check: Safely parse the index
    char* endptr;
    errno = 0;
    long index = strtol(argv[1], &endptr, 10);
    
    // Check for conversion errors
    if (endptr == argv[1] || *endptr != '\\0') {
        return "Error: Invalid index format. Index must be an integer.";
    }
    
    if (errno == ERANGE || index > INT_MAX || index < INT_MIN) {
        return "Error: Index value out of range.";
    }
    
    // Security check: Validate index bounds
    if (index < 0 || index >= argc) {
        static char error_msg[100];
        snprintf(error_msg, sizeof(error_msg), 
                "Error: Index out of bounds. Valid range: 0 to %d", argc - 1);
        return error_msg;
    }
    
    // Return the value at the specified index
    return argv[(int)index];
}

int main() {
    // Test case 1: Normal case
    char* test1[] = {"program", "2", "hello", "world"};
    printf("Test 1: %s\\n", getArgumentAtIndex(4, test1));
    
    // Test case 2: Index out of bounds
    char* test2[] = {"program", "10", "hello"};
    printf("Test 2: %s\\n", getArgumentAtIndex(3, test2));
    
    // Test case 3: Invalid index format
    char* test3[] = {"program", "abc", "hello"};
    printf("Test 3: %s\\n", getArgumentAtIndex(3, test3));
    
    // Test case 4: Insufficient arguments
    char* test4[] = {"program"};
    printf("Test 4: %s\\n", getArgumentAtIndex(1, test4));
    
    // Test case 5: Negative index
    char* test5[] = {"program", "-1", "hello"};
    printf("Test 5: %s\\n", getArgumentAtIndex(3, test5));
    
    return 0;
}
