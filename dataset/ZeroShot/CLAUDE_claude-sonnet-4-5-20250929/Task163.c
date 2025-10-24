
#include <stdio.h>

int longestRun(int* arr, int length) {
    if (arr == NULL || length == 0) {
        return 0;
    }
    
    if (length == 1) {
        return 1;
    }
    
    int maxRun = 1;
    int currentRun = 1;
    
    for (int i = 1; i < length; i++) {
        int diff = arr[i] - arr[i - 1];
        
        if (diff == 1 || diff == -1) {
            if (i > 1) {
                int prevDiff = arr[i - 1] - arr[i - 2];
                if (diff == prevDiff) {
                    currentRun++;
                } else {
                    currentRun = 2;
                }
            } else {
                currentRun++;
            }
            if (currentRun > maxRun) {
                maxRun = currentRun;
            }
        } else {
            currentRun = 1;
        }
    }
    
    return maxRun;
}

int main() {
    // Test case 1
    int test1[] = {1, 2, 3, 5, 6, 7, 8, 9};
    printf("Test 1: %d (Expected: 5)\\n", longestRun(test1, 8));
    
    // Test case 2
    int test2[] = {1, 2, 3, 10, 11, 15};
    printf("Test 2: %d (Expected: 3)\\n", longestRun(test2, 6));
    
    // Test case 3
    int test3[] = {5, 4, 2, 1};
    printf("Test 3: %d (Expected: 2)\\n", longestRun(test3, 4));
    
    // Test case 4
    int test4[] = {3, 5, 7, 10, 15};
    printf("Test 4: %d (Expected: 1)\\n", longestRun(test4, 5));
    
    // Test case 5
    int test5[] = {10, 9, 8, 7, 6, 5};
    printf("Test 5: %d (Expected: 6)\\n", longestRun(test5, 6));
    
    return 0;
}
