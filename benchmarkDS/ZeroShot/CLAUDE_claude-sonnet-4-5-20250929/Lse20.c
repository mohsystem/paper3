
#include <stdio.h>
#include <stdlib.h>

#define OFFSET 1000

int addOffset(int value) {
    return value + OFFSET;
}

int main(int argc, char* argv[]) {
    // Test cases
    printf("Test Case 1: %d (Expected: 1100)\\n", addOffset(100));
    printf("Test Case 2: %d (Expected: 1000)\\n", addOffset(0));
    printf("Test Case 3: %d (Expected: 500)\\n", addOffset(-500));
    printf("Test Case 4: %d (Expected: 3500)\\n", addOffset(2500));
    printf("Test Case 5: %d (Expected: 0)\\n", addOffset(-1000));
    
    // Read from command line if argument provided
    if (argc > 1) {
        int inputValue = atoi(argv[1]);
        int result = addOffset(inputValue);
        printf("\\nCommand line input: %d\\n", inputValue);
        printf("Result after adding offset: %d\\n", result);
    }
    
    return 0;
}
