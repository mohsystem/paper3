
#include <stdio.h>

int findOutlier(int* integers, int length) {
    int evenCount = 0;
    int oddCount = 0;
    int lastEven = 0;
    int lastOdd = 0;
    
    // Check first 3 elements to determine the majority
    int limit = length < 3 ? length : 3;
    for (int i = 0; i < limit; i++) {
        if (integers[i] % 2 == 0) {
            evenCount++;
            lastEven = integers[i];
        } else {
            oddCount++;
            lastOdd = integers[i];
        }
    }
    
    // Determine if we're looking for odd or even outlier
    int lookingForOdd = evenCount > oddCount;
    
    // If we already found the outlier in first 3 elements
    if (lookingForOdd && oddCount == 1) {
        return lastOdd;
    }
    if (!lookingForOdd && evenCount == 1) {
        return lastEven;
    }
    
    // Search the rest of the array
    for (int i = 3; i < length; i++) {
        if (lookingForOdd && integers[i] % 2 != 0) {
            return integers[i];
        }
        if (!lookingForOdd && integers[i] % 2 == 0) {
            return integers[i];
        }
    }
    
    return 0; // Should never reach here with valid input
}

int main() {
    // Test case 1
    int test1[] = {2, 4, 0, 100, 4, 11, 2602, 36};
    printf("Test 1: %d\\n", findOutlier(test1, 8)); // Expected: 11
    
    // Test case 2
    int test2[] = {160, 3, 1719, 19, 11, 13, -21};
    printf("Test 2: %d\\n", findOutlier(test2, 7)); // Expected: 160
    
    // Test case 3
    int test3[] = {1, 3, 5, 7, 9, 10};
    printf("Test 3: %d\\n", findOutlier(test3, 6)); // Expected: 10
    
    // Test case 4
    int test4[] = {2, 4, 6, 8, 10, 1};
    printf("Test 4: %d\\n", findOutlier(test4, 6)); // Expected: 1
    
    // Test case 5
    int test5[] = {-3, -5, -7, 2, -9};
    printf("Test 5: %d\\n", findOutlier(test5, 5)); // Expected: 2
    
    return 0;
}
