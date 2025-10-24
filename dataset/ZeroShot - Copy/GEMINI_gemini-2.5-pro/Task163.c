#include <stdio.h>
#include <stddef.h> // For size_t
#include <stdlib.h> // For NULL

/**
 * @brief Finds the length of the longest consecutive run in an array.
 * A consecutive run can be either increasing or decreasing.
 * 
 * @param arr A pointer to the constant integer array.
 * @param n The number of elements in the array.
 * @return The length of the longest consecutive run.
 */
int longestRun(const int* arr, size_t n) {
    if (arr == NULL || n == 0) {
        return 0;
    }
    if (n == 1) {
        return 1;
    }

    int maxLength = 1;
    int currentIncLength = 1;
    int currentDecLength = 1;

    for (size_t i = 1; i < n; ++i) {
        // Check for an increasing run
        if (arr[i] == arr[i - 1] + 1) {
            currentIncLength++;
        } else {
            currentIncLength = 1;
        }

        // Check for a decreasing run
        if (arr[i] == arr[i - 1] - 1) {
            currentDecLength++;
        } else {
            currentDecLength = 1;
        }

        // Update the maximum length found so far
        if (currentIncLength > maxLength) {
            maxLength = currentIncLength;
        }
        if (currentDecLength > maxLength) {
            maxLength = currentDecLength;
        }
    }

    return maxLength;
}

void printArray(const int* arr, size_t n) {
    printf("{");
    for (size_t i = 0; i < n; ++i) {
        printf("%d%s", arr[i], (i == n - 1 ? "" : ", "));
    }
    printf("}");
}

int main() {
    // Test Case 1: Long increasing run
    int test1[] = {1, 2, 3, 5, 6, 7, 8, 9};
    size_t n1 = sizeof(test1) / sizeof(test1[0]);
    printf("Test 1: ");
    printArray(test1, n1);
    printf(" -> Expected: 5, Got: %d\n", longestRun(test1, n1));

    // Test Case 2: Long decreasing run
    int test2[] = {9, 8, 7, 6, 3, 2, 1};
    size_t n2 = sizeof(test2) / sizeof(test2[0]);
    printf("Test 2: ");
    printArray(test2, n2);
    printf(" -> Expected: 4, Got: %d\n", longestRun(test2, n2));

    // Test Case 3: Mixed runs
    int test3[] = {1, 2, 1, 0, -1, 4, 5};
    size_t n3 = sizeof(test3) / sizeof(test3[0]);
    printf("Test 3: ");
    printArray(test3, n3);
    printf(" -> Expected: 4, Got: %d\n", longestRun(test3, n3));

    // Test Case 4: No consecutive runs
    int test4[] = {3, 5, 7, 10, 15};
    size_t n4 = sizeof(test4) / sizeof(test4[0]);
    printf("Test 4: ");
    printArray(test4, n4);
    printf(" -> Expected: 1, Got: %d\n", longestRun(test4, n4));

    // Test Case 5: Empty array
    int* test5 = NULL;
    size_t n5 = 0;
    printf("Test 5: {} -> Expected: 0, Got: %d\n", longestRun(test5, n5));

    return 0;
}