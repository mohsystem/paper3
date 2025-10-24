
#include <stdio.h>
#include <stdbool.h>

bool xorGame(int* nums, int numsSize) {
    int xorVal = 0;
    for (int i = 0; i < numsSize; i++) {
        xorVal ^= nums[i];
    }
    return xorVal == 0 || numsSize % 2 == 0;
}

int main() {
    // Test case 1
    int test1[] = {1, 1, 2};
    printf("%s\\n", xorGame(test1, 3) ? "true" : "false");
    
    // Test case 2
    int test2[] = {0, 1};
    printf("%s\\n", xorGame(test2, 2) ? "true" : "false");
    
    // Test case 3
    int test3[] = {1, 2, 3};
    printf("%s\\n", xorGame(test3, 3) ? "true" : "false");
    
    // Test case 4
    int test4[] = {1, 1};
    printf("%s\\n", xorGame(test4, 2) ? "true" : "false");
    
    // Test case 5
    int test5[] = {2, 2, 2, 2};
    printf("%s\\n", xorGame(test5, 4) ? "true" : "false");
    
    return 0;
}
