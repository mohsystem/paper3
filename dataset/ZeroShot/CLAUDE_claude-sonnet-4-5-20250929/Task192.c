
#include <stdio.h>
#include <stdbool.h>

bool xorGame(int* nums, int numsSize) {
    int xorSum = 0;
    for (int i = 0; i < numsSize; i++) {
        xorSum ^= nums[i];
    }
    return xorSum == 0 || numsSize % 2 == 0;
}

int main() {
    int test1[] = {1, 1, 2};
    printf("%s\\n", xorGame(test1, 3) ? "true" : "false"); // false
    
    int test2[] = {0, 1};
    printf("%s\\n", xorGame(test2, 2) ? "true" : "false"); // true
    
    int test3[] = {1, 2, 3};
    printf("%s\\n", xorGame(test3, 3) ? "true" : "false"); // true
    
    int test4[] = {1, 1};
    printf("%s\\n", xorGame(test4, 2) ? "true" : "false"); // true
    
    int test5[] = {2, 2, 2, 2};
    printf("%s\\n", xorGame(test5, 4) ? "true" : "false"); // true
    
    return 0;
}
