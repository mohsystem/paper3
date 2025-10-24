#include <stdio.h>
#include <stddef.h> // Required for size_t

/**
 * @brief Given an array of ones and zeroes, convert the equivalent binary value to an integer.
 * 
 * @param binary A pointer to a constant integer array (0s and 1s).
 * @param size The number of elements in the array.
 * @return The integer representation of the binary value.
 */
int binaryArrayToNumber(const int* binary, size_t size) {
    int number = 0;
    for (size_t i = 0; i < size; ++i) {
        // Left shift the current number by 1 and add the new bit using bitwise OR.
        // This is equivalent to number = number * 2 + binary[i];
        number = (number << 1) | binary[i];
    }
    return number;
}

int main() {
    // Test Case 1
    int arr1[] = {0, 0, 0, 1};
    size_t size1 = sizeof(arr1) / sizeof(arr1[0]);
    printf("%d\n", binaryArrayToNumber(arr1, size1)); // Expected: 1

    // Test Case 2
    int arr2[] = {0, 0, 1, 0};
    size_t size2 = sizeof(arr2) / sizeof(arr2[0]);
    printf("%d\n", binaryArrayToNumber(arr2, size2)); // Expected: 2

    // Test Case 3
    int arr3[] = {0, 1, 0, 1};
    size_t size3 = sizeof(arr3) / sizeof(arr3[0]);
    printf("%d\n", binaryArrayToNumber(arr3, size3)); // Expected: 5

    // Test Case 4
    int arr4[] = {1, 0, 0, 1};
    size_t size4 = sizeof(arr4) / sizeof(arr4[0]);
    printf("%d\n", binaryArrayToNumber(arr4, size4)); // Expected: 9

    // Test Case 5
    int arr5[] = {1, 1, 1, 1};
    size_t size5 = sizeof(arr5) / sizeof(arr5[0]);
    printf("%d\n", binaryArrayToNumber(arr5, size5)); // Expected: 15

    return 0;
}