#include <stdio.h>
#include <limits.h> // For INT_MIN

/**
 * Returns the value at a specific index in an array.
 * Performs bounds checking to prevent out-of-bounds memory access.
 *
 * @param arr The source array of integers.
 * @param size The number of elements in the array.
 * @param index The index of the element to retrieve.
 * @return The integer value at the specified index, or INT_MIN if the index is out of bounds.
 */
int getValueAtIndex(const int arr[], int size, int index) {
    // Security: Validate the index against the array size. This is crucial in C
    // to prevent buffer overflows and other memory-related vulnerabilities that
    // arise from accessing an array out of its bounds.
    if (index < 0 || index >= size) {
        fprintf(stderr, "Error: Index %d is out of bounds.\n", index);
        return INT_MIN; // Return a sentinel value to indicate an error.
    }
    return arr[index];
}

int main() {
    int data[] = {10, 20, 30, 40, 50, 60, 70};
    int size = sizeof(data) / sizeof(data[0]);

    // --- 5 Test Cases ---

    // Test Case 1: Valid index in the middle
    int index1 = 3;
    int value1 = getValueAtIndex(data, size, index1);
    if (value1 != INT_MIN) {
        printf("Test 1: Value at index %d is %d\n", index1, value1);
    }

    // Test Case 2: Valid index at the beginning
    int index2 = 0;
    int value2 = getValueAtIndex(data, size, index2);
    if (value2 != INT_MIN) {
        printf("Test 2: Value at index %d is %d\n", index2, value2);
    }

    // Test Case 3: Valid index at the end
    int index3 = size - 1;
    int value3 = getValueAtIndex(data, size, index3);
    if (value3 != INT_MIN) {
        printf("Test 3: Value at index %d is %d\n", index3, value3);
    }

    // Test Case 4: Invalid index (negative)
    int index4 = -1;
    printf("Test 4: ");
    getValueAtIndex(data, size, index4);

    // Test Case 5: Invalid index (too large)
    int index5 = 10;
    printf("Test 5: ");
    getValueAtIndex(data, size, index5);

    return 0;
}