
#include <stdio.h>
#include <stdlib.h>

int binaryArrayToInt(const int* binaryArray, int length) {
    // Security: Validate input is not NULL
    if (binaryArray == NULL) {
        fprintf(stderr, "Error: Input array cannot be NULL\\n");
        return -1;
    }
    
    // Security: Check for empty array
    if (length <= 0) {
        return 0;
    }
    
    // Security: Validate array length to prevent integer overflow
    if (length > 31) {
        fprintf(stderr, "Error: Binary array too long, may cause integer overflow\\n");
        return -1;
    }
    
    int result = 0;
    
    // Security: Validate each element and convert binary to integer
    for (int i = 0; i < length; i++) {
        // Security: Ensure only 0 or 1 values are present
        if (binaryArray[i] != 0 && binaryArray[i] != 1) {
            fprintf(stderr, "Error: Array must contain only 0 or 1 values\\n");
            return -1;
        }
        
        // Shift left and add current bit
        result = (result << 1) | binaryArray[i];
    }
    
    return result;
}

int main() {
    // Test case 1
    int test1[] = {0, 0, 0, 1};
    printf("Test 1: [0, 0, 0, 1] => %d\\n", binaryArrayToInt(test1, 4));
    
    // Test case 2
    int test2[] = {0, 0, 1, 0};
    printf("Test 2: [0, 0, 1, 0] => %d\\n", binaryArrayToInt(test2, 4));
    
    // Test case 3
    int test3[] = {0, 1, 0, 1};
    printf("Test 3: [0, 1, 0, 1] => %d\\n", binaryArrayToInt(test3, 4));
    
    // Test case 4
    int test4[] = {1, 0, 0, 1};
    printf("Test 4: [1, 0, 0, 1] => %d\\n", binaryArrayToInt(test4, 4));
    
    // Test case 5
    int test5[] = {1, 1, 1, 1};
    printf("Test 5: [1, 1, 1, 1] => %d\\n", binaryArrayToInt(test5, 4));
    
    return 0;
}
