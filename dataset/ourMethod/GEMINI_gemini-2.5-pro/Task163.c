#include <stdio.h>
#include <stddef.h>

/**
 * @brief Helper function to find the maximum of two integers.
 * @param a First integer.
 * @param b Second integer.
 * @return The larger of the two integers.
 */
int max(int a, int b) {
    return a > b ? a : b;
}

/**
 * @brief Calculates the length of the longest consecutive-run in an array.
 * A consecutive-run is a list of adjacent, consecutive integers,
 * which can be either increasing or decreasing.
 *
 * @param arr The input array of integers.
 * @param size The number of elements in the array.
 * @return The length of the longest consecutive-run.
 */
int longestRun(const int arr[], size_t size) {
    if (size == 0) {
        return 0;
    }
    if (size == 1) {
        return 1;
    }

    int maxLength = 1;
    int currentIncreasingLength = 1;
    int currentDecreasingLength = 1;

    for (size_t i = 1; i < size; i++) {
        if (arr[i] == arr[i - 1] + 1) {
            currentIncreasingLength++;
            currentDecreasingLength = 1;
        } else if (arr[i] == arr[i - 1] - 1) {
            currentDecreasingLength++;
            currentIncreasingLength = 1;
        } else {
            currentIncreasingLength = 1;
            currentDecreasingLength = 1;
        }
        maxLength = max(maxLength, max(currentIncreasingLength, currentDecreasingLength));
    }
    return maxLength;
}

int main() {
    // Test cases
    int test1[] = {1, 2, 3, 5, 6, 7, 8, 9};
    size_t size1 = sizeof(test1) / sizeof(test1[0]);
    printf("longestRun([1, 2, 3, 5, 6, 7, 8, 9]) -> %d\n", longestRun(test1, size1));

    int test2[] = {1, 2, 3, 10, 11, 15};
    size_t size2 = sizeof(test2) / sizeof(test2[0]);
    printf("longestRun([1, 2, 3, 10, 11, 15]) -> %d\n", longestRun(test2, size2));

    int test3[] = {5, 4, 2, 1};
    size_t size3 = sizeof(test3) / sizeof(test3[0]);
    printf("longestRun([5, 4, 2, 1]) -> %d\n", longestRun(test3, size3));

    int test4[] = {3, 5, 7, 10, 15};
    size_t size4 = sizeof(test4) / sizeof(test4[0]);
    printf("longestRun([3, 5, 7, 10, 15]) -> %d\n", longestRun(test4, size4));
    
    int test5[] = {1, 2, 3, 2, 1, 0, 1, 2};
    size_t size5 = sizeof(test5) / sizeof(test5[0]);
    printf("longestRun([1, 2, 3, 2, 1, 0, 1, 2]) -> %d\n", longestRun(test5, size5));

    return 0;
}