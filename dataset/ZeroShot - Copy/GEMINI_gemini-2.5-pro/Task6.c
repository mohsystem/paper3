#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

/**
 * @brief Finds the unique number in an array where all other numbers are equal.
 *
 * It's guaranteed that the array contains at least 3 numbers.
 *
 * @param arr The input array of doubles.
 * @param n The size of the array.
 * @return The unique number in the array.
 */
double findUniq(const double arr[], size_t n) {
    // Although guaranteed, adding a check for size is a secure coding practice.
    if (n < 3) {
         fprintf(stderr, "Error: Array must contain at least 3 numbers.\n");
         exit(EXIT_FAILURE);
    }

    // Compare the first two elements. If they are different, the unique
    // number is one of them. The third element decides which one.
    if (arr[0] != arr[1]) {
        return (arr[0] == arr[2]) ? arr[1] : arr[0];
    } else {
        // If the first two are the same, this is the common number.
        // Iterate through the rest to find the different one.
        double commonNumber = arr[0];
        for (size_t i = 2; i < n; ++i) {
            if (arr[i] != commonNumber) {
                return arr[i];
            }
        }
    }
    
    // This part should be unreachable given the problem constraints.
    // In C, exiting with an error is a common way to handle fatal logic errors.
    fprintf(stderr, "Error: No unique number found, which violates problem constraints.\n");
    exit(EXIT_FAILURE);
}

int main() {
    // Test Case 1
    double arr1[] = {1, 1, 1, 2, 1, 1};
    printf("Test 1 Result: %g\n", findUniq(arr1, sizeof(arr1)/sizeof(arr1[0])));

    // Test Case 2
    double arr2[] = {0, 0, 0.55, 0, 0};
    printf("Test 2 Result: %g\n", findUniq(arr2, sizeof(arr2)/sizeof(arr2[0])));

    // Test Case 3: Unique at the beginning
    double arr3[] = {3, 1, 1, 1, 1, 1};
    printf("Test 3 Result: %g\n", findUniq(arr3, sizeof(arr3)/sizeof(arr3[0])));

    // Test Case 4: Unique at the end
    double arr4[] = {4, 4, 4, 4, 3.5};
    printf("Test 4 Result: %g\n", findUniq(arr4, sizeof(arr4)/sizeof(arr4[0])));
    
    // Test Case 5: Negative numbers
    double arr5[] = {-1, -1, -1, -2, -1};
    printf("Test 5 Result: %g\n", findUniq(arr5, sizeof(arr5)/sizeof(arr5[0])));

    return 0;
}