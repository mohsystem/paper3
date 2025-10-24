
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
    int test1[] = {1, 1, 2};
    printf("%d\\n", xorGame(test1, 3)); // 0 (false)
    
    int test2[] = {0, 1};
    printf("%d\\n", xorGame(test2, 2)); // 1 (true)
    
    int test3[] = {1, 2, 3};
    printf("%d\\n", xorGame(test3, 3)); // 1 (true)
    
    int test4[] = {1};
    printf("%d\\n", xorGame(test4, 1)); // 1 (true)
    
    int test5[] = {2, 2, 2, 2};
    printf("%d\\n", xorGame(test5, 4)); // 1 (true)
    
    return 0;
}
