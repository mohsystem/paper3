#include <stdio.h>
#include <stddef.h> // for size_t

/**
 * Converts an array of binary digits (0s and 1s) to its integer equivalent.
 * @param arr A pointer to a constant integer array.
 * @param size The number of elements in the array.
 * @return The decimal integer value of the binary representation.
 */
int binary_array_to_number(const int* arr, size_t size) {
    int number = 0;
    for (size_t i = 0; i < size; ++i) {
        // Left shift the current number by 1 and add the new bit.
        // This is equivalent to number = number * 2 + arr[i];
        number = (number << 1) | arr[i];
    }
    return number;
}

int main() {
    // Test cases
    int arr1[] = {0, 0, 0, 1};
    size_t size1 = sizeof(arr1) / sizeof(arr1[0]);
    printf("%d\n", binary_array_to_number(arr1, size1)); // Expected: 1

    int arr2[] = {0, 1, 0, 1};
    size_t size2 = sizeof(arr2) / sizeof(arr2[0]);
    printf("%d\n", binary_array_to_number(arr2, size2)); // Expected: 5

    int arr3[] = {1, 0, 0, 1};
    size_t size3 = sizeof(arr3) / sizeof(arr3[0]);
    printf("%d\n", binary_array_to_number(arr3, size3)); // Expected: 9

    int arr4[] = {1, 1, 1, 1};
    size_t size4 = sizeof(arr4) / sizeof(arr4[0]);
    printf("%d\n", binary_array_to_number(arr4, size4)); // Expected: 15
    
    int arr5[] = {1, 0, 1, 1};
    size_t size5 = sizeof(arr5) / sizeof(arr5[0]);
    printf("%d\n", binary_array_to_number(arr5, size5)); // Expected: 11

    return 0;
}