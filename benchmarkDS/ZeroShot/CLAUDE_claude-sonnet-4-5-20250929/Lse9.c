
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* getArgumentAtIndex(char** args, int argsCount, int index) {
    if (args == NULL || index < 0 || index >= argsCount) {
        return "Error: Invalid index or no argument at the specified index";
    }
    return args[index];
}

int main(int argc, char* argv[]) {
    // Test cases
    char* testArgs1[] = {"arg0", "arg1", "arg2", "arg3", "arg4"};
    int testArgs1Count = 5;
    
    // Test case 1: Get argument at index 1
    printf("Test 1 - Index 1: %s\\n", getArgumentAtIndex(testArgs1, testArgs1Count, 1));
    
    // Test case 2: Get argument at index 3
    printf("Test 2 - Index 3: %s\\n", getArgumentAtIndex(testArgs1, testArgs1Count, 3));
    
    // Test case 3: Get argument at index 0
    printf("Test 3 - Index 0: %s\\n", getArgumentAtIndex(testArgs1, testArgs1Count, 0));
    
    // Test case 4: Invalid index (out of bounds)
    printf("Test 4 - Index 10: %s\\n", getArgumentAtIndex(testArgs1, testArgs1Count, 10));
    
    // Test case 5: Negative index
    printf("Test 5 - Index -1: %s\\n", getArgumentAtIndex(testArgs1, testArgs1Count, -1));
    
    // If actual command line arguments are provided
    if (argc > 1) {
        char* endptr;
        long index = strtol(argv[1], &endptr, 10);
        
        if (*endptr == '\\0' && endptr != argv[1]) {
            // Valid integer conversion
            char** remainingArgs = &argv[2];
            int remainingArgsCount = argc - 2;
            printf("\\nActual argument at index %ld: %s\\n", 
                   index, getArgumentAtIndex(remainingArgs, remainingArgsCount, (int)index));
        } else {
            printf("Error: First argument must be a valid integer\\n");
        }
    }
    
    return 0;
}
