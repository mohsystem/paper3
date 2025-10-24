#include <stdio.h>
#include <stdbool.h> 
#include <stddef.h>  

/**
 * Finds the outlier integer in an array that is otherwise comprised of either all even or all odd integers.
 *
 * @param integers Pointer to the input array of integers.
 * @param count The number of elements in the array (will be at least 3).
 * @return The outlier integer N.
 */
int findOutlier(const int* integers, size_t count) {
    // According to the prompt, the count is at least 3, so direct access is safe.

    int evenCount = 0;
    // Check the first three elements to determine the majority parity.
    for (size_t i = 0; i < 3; ++i) {
        if (integers[i] % 2 == 0) {
            evenCount++;
        }
    }

    bool majorityIsEven = (evenCount >= 2);

    // Iterate through the array to find the single number with the opposite parity.
    for (size_t i = 0; i < count; ++i) {
        int num = integers[i];
        if (majorityIsEven) {
            // If the majority is even, we are looking for the single odd number.
            if (num % 2 != 0) {
                return num;
            }
        } else {
            // If the majority is odd, we are looking for the single even number.
            if (num % 2 == 0) {
                return num;
            }
        }
    }
    
    // This part should be unreachable given the problem constraints.
    return 0; 
}

void runTest(const char* testName, const int* testData, size_t size) {
    printf("%s: [", testName);
    for(size_t i = 0; i < size; ++i) {
        printf("%d%s", testData[i], (i == size - 1 ? "" : ", "));
    }
    printf("] -> %d\n", findOutlier(testData, size));
}

int main() {
    // Test cases
    int test1[] = {2, 4, 0, 100, 4, 11, 2602, 36};
    runTest("Test 1", test1, sizeof(test1)/sizeof(test1[0]));

    int test2[] = {160, 3, 1719, 19, 11, 13, -21};
    runTest("Test 2", test2, sizeof(test2)/sizeof(test2[0]));

    int test3[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2};
    runTest("Test 3", test3, sizeof(test3)/sizeof(test3[0]));

    int test4[] = {2, 2, 2, 2, 3, 2, 2, 2};
    runTest("Test 4", test4, sizeof(test4)/sizeof(test4[0]));

    int test5[] = {1, -3, 5, 7, 9, -12, 11, 13};
    runTest("Test 5", test5, sizeof(test5)/sizeof(test5[0]));
    
    return 0;
}