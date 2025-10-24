
#include <stdio.h>

int binaryArrayToInt(int binary[], int length) {
    int result = 0;
    for (int i = 0; i < length; i++) {
        result = result * 2 + binary[i];
    }
    return result;
}

int main() {
    // Test case 1
    int test1[] = {0, 0, 0, 1};
    printf("Testing: [0, 0, 0, 1] ==> %d\\n", binaryArrayToInt(test1, 4));
    
    // Test case 2
    int test2[] = {0, 0, 1, 0};
    printf("Testing: [0, 0, 1, 0] ==> %d\\n", binaryArrayToInt(test2, 4));
    
    // Test case 3
    int test3[] = {0, 1, 0, 1};
    printf("Testing: [0, 1, 0, 1] ==> %d\\n", binaryArrayToInt(test3, 4));
    
    // Test case 4
    int test4[] = {1, 0, 0, 1};
    printf("Testing: [1, 0, 0, 1] ==> %d\\n", binaryArrayToInt(test4, 4));
    
    // Test case 5
    int test5[] = {1, 1, 1, 1};
    printf("Testing: [1, 1, 1, 1] ==> %d\\n", binaryArrayToInt(test5, 4));
    
    return 0;
}
