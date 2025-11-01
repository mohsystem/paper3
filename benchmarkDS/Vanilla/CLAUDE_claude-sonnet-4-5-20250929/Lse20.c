
#include <stdio.h>
#include <stdlib.h>

#define OFFSET 1000

int addOffset(int value) {
    return value + OFFSET;
}

int main(int argc, char* argv[]) {
    // Test case 1
    printf("Test 1: Input = 5, Output = %d\\n", addOffset(5));
    
    // Test case 2
    printf("Test 2: Input = 100, Output = %d\\n", addOffset(100));
    
    // Test case 3
    printf("Test 3: Input = -500, Output = %d\\n", addOffset(-500));
    
    // Test case 4
    printf("Test 4: Input = 0, Output = %d\\n", addOffset(0));
    
    // Test case 5
    printf("Test 5: Input = 2500, Output = %d\\n", addOffset(2500));
    
    // If command line argument is provided
    if (argc > 1) {
        int inputValue = atoi(argv[1]);
        int result = addOffset(inputValue);
        printf("Command line input: %d, Result: %d\\n", inputValue, result);
    }
    
    return 0;
}
