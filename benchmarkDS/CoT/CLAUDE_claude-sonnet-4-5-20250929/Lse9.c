
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* getArgumentAtIndex(char* args[], int argc, int index) {
    // Validate index is non-negative
    if (index < 0) {
        return "Error: Index cannot be negative";
    }
    
    // Check if index is within bounds
    if (index >= argc) {
        return "Error: Index out of bounds";
    }
    
    return args[index];
}

int main(int argc, char* argv[]) {
    // Test cases
    printf("Test Case 1:\\n");
    char* test1[] = {"arg0", "arg1", "arg2", "arg3", "arg4"};
    printf("%s\\n", getArgumentAtIndex(test1, 5, 1));
    
    printf("\\nTest Case 2:\\n");
    char* test2[] = {"arg0", "arg1", "arg2", "arg3", "arg4"};
    printf("%s\\n", getArgumentAtIndex(test2, 5, 3));
    
    printf("\\nTest Case 3:\\n");
    char* test3[] = {"arg0", "arg1"};
    printf("%s\\n", getArgumentAtIndex(test3, 2, 0));
    
    printf("\\nTest Case 4:\\n");
    char* test4[] = {"arg0", "arg1", "arg2"};
    printf("%s\\n", getArgumentAtIndex(test4, 3, 5));
    
    printf("\\nTest Case 5:\\n");
    char* test5[] = {"arg0", "arg1", "arg2"};
    printf("%s\\n", getArgumentAtIndex(test5, 3, -1));
    
    // Real command line usage
    if (argc > 1) {
        char* endptr;
        long index = strtol(argv[1], &endptr, 10);
        
        if (*endptr != '\\0') {
            printf("Error: First argument must be a valid integer\\n");
        } else {
            const char* result = getArgumentAtIndex(argv, argc, (int)index);
            printf("\\nCommand line result: %s\\n", result);
        }
    }
    
    return 0;
}
