#include <stdio.h>

/**
 * Securely gets the value at a specified index of an integer array.
 *
 * @param arr Pointer to the array.
 * @param size The number of elements in the array.
 * @param index The index of the element to retrieve.
 * @param out_value Pointer to an integer where the result will be stored.
 * @return 0 on success, -1 on failure (e.g., null pointers or out-of-bounds index).
 */
int getValueAtIndex(const int* arr, size_t size, int index, int* out_value) {
    // Secure: Check for null pointers and if the index is within the valid range.
    if (arr == NULL || out_value == NULL) {
        return -1; // Indicate error: null pointer
    }
    if (index < 0 || (size_t)index >= size) {
        return -1; // Indicate error: index out of bounds
    }

    *out_value = arr[index];
    return 0; // Indicate success
}

void run_test(const int* data, size_t size, int index) {
    int value;
    if (getValueAtIndex(data, size, index, &value) == 0) {
        printf("Test case index: %d -> Success! Value: %d\n", index, value);
    } else {
        printf("Test case index: %d -> Failed! Index is out of bounds or null pointer provided.\n", index);
    }
}

int main() {
    int data[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    size_t size = sizeof(data) / sizeof(data[0]);

    printf("Array: [10, 20, 30, 40, 50, 60, 70, 80, 90, 100]\n");

    // Test Cases
    run_test(data, size, 5);  // Valid case
    run_test(data, size, 0);  // Valid edge case (first element)
    run_test(data, size, 9);  // Valid edge case (last element)
    run_test(data, size, -1); // Invalid case (negative index)
    run_test(data, size, 10); // Invalid case (index equal to size)

    return 0;
}