
#include <stdio.h>
#include <stdlib.h>

int findOutlier(int* array, int length) {
    // Security: Validate input to prevent errors
    if (array == NULL || length < 3) {
        fprintf(stderr, "Error: Array must not be NULL and must have at least 3 elements\\n");
        exit(EXIT_FAILURE);
    }
    
    // Count even and odd numbers in first 3 elements to determine the pattern
    int evenCount = 0;
    int oddCount = 0;
    
    // Check first 3 elements to determine if we're looking for even or odd outlier\n    for (int i = 0; i < 3; i++) {\n        if (array[i] % 2 == 0) {\n            evenCount++;\n        } else {\n            oddCount++;\n        }\n    }\n    \n    // Determine if we're looking for the odd or even outlier
    int lookingForOdd = evenCount > oddCount;
    
    // Find and return the outlier
    for (int i = 0; i < length; i++) {
        if (lookingForOdd && array[i] % 2 != 0) {
            return array[i];
        } else if (!lookingForOdd && array[i] % 2 == 0) {
            return array[i];
        }
    }
    
    // This should never be reached given valid input
    fprintf(stderr, "Error: No outlier found in array\\n");
    exit(EXIT_FAILURE);
}

int main() {
    // Test case 1: Only odd number
    int test1[] = {2, 4, 0, 100, 4, 11, 2602, 36};
    printf("Test 1: %d\\n", findOutlier(test1, 8)); // Expected: 11
    
    // Test case 2: Only even number
    int test2[] = {160, 3, 1719, 19, 11, 13, -21};
    printf("Test 2: %d\\n", findOutlier(test2, 7)); // Expected: 160
    
    // Test case 3: Negative odd outlier
    int test3[] = {2, 4, 6, 8, 10, -1};
    printf("Test 3: %d\\n", findOutlier(test3, 6)); // Expected: -1
    
    // Test case 4: Negative even outlier
    int test4[] = {1, 3, 5, 7, -2, 9};
    printf("Test 4: %d\\n", findOutlier(test4, 6)); // Expected: -2
    
    // Test case 5: Zero as outlier
    int test5[] = {1, 3, 5, 0, 7, 9};
    printf("Test 5: %d\\n", findOutlier(test5, 6)); // Expected: 0
    
    return 0;
}
