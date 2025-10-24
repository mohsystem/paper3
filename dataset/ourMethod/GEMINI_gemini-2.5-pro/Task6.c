#include <stdio.h>
#include <stddef.h>
#include <math.h>

/**
 * @brief Finds the unique number in an array where all other numbers are equal.
 * 
 * It is guaranteed that the array contains at least 3 numbers.
 * 
 * @param arr A pointer to a constant array of doubles.
 * @param n The size of the array.
 * @return The unique double in the array. Returns NAN on invalid input.
 */
double findUniq(const double* arr, size_t n) {
    // For robustness, check for invalid inputs, although problem guarantees n >= 3.
    if (arr == NULL || n < 3) {
        return NAN;
    }

    // If the first two elements are equal, they represent the common number.
    // We then iterate from the third element to find the one that is different.
    if (arr[0] == arr[1]) {
        double common_number = arr[0];
        for (size_t i = 2; i < n; ++i) {
            if (arr[i] != common_number) {
                return arr[i];
            }
        }
    } else {
        // If the first two elements are different, one of them is the unique number.
        // The third element will be equal to the common number, which resolves the ambiguity.
        if (arr[0] == arr[2]) {
            // arr[0] and arr[2] are the common number, so arr[1] is unique.
            return arr[1];
        } else {
            // arr[1] and arr[2] must be the common number, so arr[0] is unique.
            return arr[0];
        }
    }

    // This code should be unreachable given the problem constraints.
    // Return NaN (Not a Number) to indicate failure if the constraints are violated.
    return NAN;
}

int main() {
    // Test Case 1
    double test1[] = { 1, 1, 1, 2, 1, 1 };
    size_t n1 = sizeof(test1) / sizeof(test1[0]);
    printf("Test 1: Unique in { 1, 1, 1, 2, 1, 1 } is %f\n", findUniq(test1, n1));

    // Test Case 2
    double test2[] = { 0, 0, 0.55, 0, 0 };
    size_t n2 = sizeof(test2) / sizeof(test2[0]);
    printf("Test 2: Unique in { 0, 0, 0.55, 0, 0 } is %f\n", findUniq(test2, n2));

    // Test Case 3
    double test3[] = { 3, 1, 1, 1, 1 };
    size_t n3 = sizeof(test3) / sizeof(test3[0]);
    printf("Test 3: Unique in { 3, 1, 1, 1, 1 } is %f\n", findUniq(test3, n3));

    // Test Case 4
    double test4[] = { 1, 3, 1, 1, 1 };
    size_t n4 = sizeof(test4) / sizeof(test4[0]);
    printf("Test 4: Unique in { 1, 3, 1, 1, 1 } is %f\n", findUniq(test4, n4));

    // Test Case 5
    double test5[] = { -2, -2, -2, -1 };
    size_t n5 = sizeof(test5) / sizeof(test5[0]);
    printf("Test 5: Unique in { -2, -2, -2, -1 } is %f\n", findUniq(test5, n5));
    
    return 0;
}