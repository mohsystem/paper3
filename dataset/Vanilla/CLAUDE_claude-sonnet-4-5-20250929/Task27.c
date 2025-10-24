
#include <stdio.h>

int findOutlier(int integers[], int length) {
    int evenCount = 0;
    int oddCount = 0;
    int lastEven = 0;
    int lastOdd = 0;
    
    // Check first 3 elements to determine which type is the outlier
    for (int i = 0; i < 3; i++) {
        if (integers[i] % 2 == 0) {
            evenCount++;
            lastEven = integers[i];
        } else {
            oddCount++;
            lastOdd = integers[i];
        }
    }
    
    // Determine if we're looking for even or odd outlier
    int lookingForEven = evenCount < oddCount;
    
    // If we already found the outlier in first 3 elements
    if (lookingForEven && evenCount == 1) {
        return lastEven;
    }
    if (!lookingForEven && oddCount == 1) {
        return lastOdd;
    }
    
    // Search rest of array
    for (int i = 3; i < length; i++) {
        if (lookingForEven) {
            if (integers[i] % 2 == 0) {
                return integers[i];
            }
        } else {
            if (integers[i] % 2 != 0) {
                return integers[i];
            }
        }
    }
    
    return 0;
}

int main() {
    // Test case 1
    int test1[] = {2, 4, 0, 100, 4, 11, 2602, 36};
    printf("Test 1: %d\\n", findOutlier(test1, 8)); // Expected: 11
    
    // Test case 2
    int test2[] = {160, 3, 1719, 19, 11, 13, -21};
    printf("Test 2: %d\\n", findOutlier(test2, 7)); // Expected: 160
    
    // Test case 3
    int test3[] = {2, 4, 6, 8, 10, 3};
    printf("Test 3: %d\\n", findOutlier(test3, 6)); // Expected: 3
    
    // Test case 4
    int test4[] = {1, 3, 5, 7, 9, 2};
    printf("Test 4: %d\\n", findOutlier(test4, 6)); // Expected: 2
    
    // Test case 5
    int test5[] = {-1, -3, -5, 0};
    printf("Test 5: %d\\n", findOutlier(test5, 4)); // Expected: 0
    
    return 0;
}
