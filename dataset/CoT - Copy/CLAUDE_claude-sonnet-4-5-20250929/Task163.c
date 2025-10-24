
#include <stdio.h>
#include <limits.h>

int longestRun(int arr[], int size) {
    // Input validation
    if (arr == NULL || size == 0) {
        return 0;
    }
    
    if (size == 1) {
        return 1;
    }
    
    int maxLength = 1;
    int currentLength = 1;
    
    for (int i = 1; i < size; i++) {
        // Check for overflow when computing difference
        long long diff = (long long)arr[i] - (long long)arr[i-1];
        
        // Check if consecutive (increasing or decreasing by 1)
        if (diff == 1 || diff == -1) {
            currentLength++;
            if (currentLength > maxLength) {
                maxLength = currentLength;
            }
        } else {
            currentLength = 1;
        }
    }
    
    return maxLength;
}

int main() {
    // Test case 1
    int test1[] = {1, 2, 3, 5, 6, 7, 8, 9};
    printf("Test 1: %d\\n", longestRun(test1, 8)); // Expected: 5
    
    // Test case 2
    int test2[] = {1, 2, 3, 10, 11, 15};
    printf("Test 2: %d\\n", longestRun(test2, 6)); // Expected: 3
    
    // Test case 3
    int test3[] = {5, 4, 2, 1};
    printf("Test 3: %d\\n", longestRun(test3, 4)); // Expected: 2
    
    // Test case 4
    int test4[] = {3, 5, 7, 10, 15};
    printf("Test 4: %d\\n", longestRun(test4, 5)); // Expected: 1
    
    // Test case 5
    int test5[] = {10, 9, 8, 7, 6, 5};
    printf("Test 5: %d\\n", longestRun(test5, 6)); // Expected: 6
    
    return 0;
}
