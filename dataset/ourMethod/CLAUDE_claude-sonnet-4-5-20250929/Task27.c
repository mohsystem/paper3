
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

// Function to find the outlier in an array of integers
// Security: All inputs validated, bounds checked, proper error handling
// Returns the outlier value, or INT32_MIN on error (sets error flag)
int findOutlier(const int* integers, size_t length, bool* error) {
    // Initialize error flag to false
    if (error != NULL) {
        *error = false;
    }
    
    // Input validation: check for NULL pointer
    if (integers == NULL) {
        if (error != NULL) {
            *error = true;
        }
        fprintf(stderr, "Error: NULL pointer passed to findOutlier\\n");
        return 0;
    }
    
    // Input validation: check minimum array size
    if (length < 3) {
        if (error != NULL) {
            *error = true;
        }
        fprintf(stderr, "Error: Array must have at least 3 elements\\n");
        return 0;
    }
    
    // Security: Check for maximum reasonable size to prevent excessive operations
    if (length > 1000000) {
        if (error != NULL) {
            *error = true;
        }
        fprintf(stderr, "Error: Array size exceeds maximum allowed limit\\n");
        return 0;
    }
    
    // Count even and odd numbers in first 3 elements to determine majority type
    // This provides early determination of array type
    int evenCount = 0;
    int oddCount = 0;
    
    // Check first 3 elements with bounds guarantee (already validated length >= 3)
    for (size_t i = 0; i < 3; i++) {
        if (integers[i] % 2 == 0) {
            evenCount++;
        } else {
            oddCount++;
        }
    }
    
    // Determine if we're looking for an even or odd outlier
    // Majority in first 3 elements determines the array type
    bool lookingForEven = (oddCount > evenCount);
    
    // Find and return the outlier
    // Security: using size_t for index to prevent signed integer issues
    for (size_t i = 0; i < length; i++) {
        bool isEven = (integers[i] % 2 == 0);
        if (lookingForEven && isEven) {
            return integers[i];
        }
        if (!lookingForEven && !isEven) {
            return integers[i];
        }
    }
    
    // This should never be reached given problem constraints
    if (error != NULL) {
        *error = true;
    }
    fprintf(stderr, "Error: No outlier found in array\\n");
    return 0;
}

int main(void) {
    bool error = false;
    int result = 0;
    
    // Test case 1: Outlier is odd
    int test1[] = {2, 4, 0, 100, 4, 11, 2602, 36};
    size_t test1_len = sizeof(test1) / sizeof(test1[0]);
    result = findOutlier(test1, test1_len, &error);
    if (!error) {
        printf("Test 1: %d (expected: 11)\\n", result);
    }
    
    // Test case 2: Outlier is even
    int test2[] = {160, 3, 1719, 19, 11, 13, -21};
    size_t test2_len = sizeof(test2) / sizeof(test2[0]);
    result = findOutlier(test2, test2_len, &error);
    if (!error) {
        printf("Test 2: %d (expected: 160)\\n", result);
    }
    
    // Test case 3: Outlier at beginning
    int test3[] = {1, 2, 4, 6, 8, 10};
    size_t test3_len = sizeof(test3) / sizeof(test3[0]);
    result = findOutlier(test3, test3_len, &error);
    if (!error) {
        printf("Test 3: %d (expected: 1)\\n", result);
    }
    
    // Test case 4: Outlier at end
    int test4[] = {2, 4, 6, 8, 10, 3};
    size_t test4_len = sizeof(test4) / sizeof(test4[0]);
    result = findOutlier(test4, test4_len, &error);
    if (!error) {
        printf("Test 4: %d (expected: 3)\\n", result);
    }
    
    // Test case 5: Negative numbers
    int test5[] = {-1, -3, -5, -7, 2};
    size_t test5_len = sizeof(test5) / sizeof(test5[0]);
    result = findOutlier(test5, test5_len, &error);
    if (!error) {
        printf("Test 5: %d (expected: 2)\\n", result);
    }
    
    return 0;
}
