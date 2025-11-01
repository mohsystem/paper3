#include <stdio.h>

/**
 * Gets a value from an array at a specified index.
 * Securely checks if the index is within the valid bounds of the array.
 *
 * @param arr Pointer to the source array of integers.
 * @param size The size of the array.
 * @param index The index from which to retrieve the value.
 * @return The value at the given index, or -1 if the index is out of bounds.
 */
int getValueFromArray(const int* arr, int size, int index) {
    // Secure: Validate that the pointer is not null and the index is within bounds.
    // This prevents segmentation faults from null pointers or out-of-bounds access.
    if (arr == NULL || index < 0 || index >= size) {
        fprintf(stderr, "Error: Index %d is out of bounds for array of size %d.\n", index, size);
        return -1; // Return a designated error code.
    }
    return arr[index];
}

int main() {
    int data[] = {10, 20, 30, 40, 50};
    int size = sizeof(data) / sizeof(data[0]);

    printf("Running 5 test cases...\n");

    // Test Case 1: Valid index in the middle
    int index1 = 2;
    printf("\nTest Case 1: index = %d\n", index1);
    int value1 = getValueFromArray(data, size, index1);
    printf("Value returned: %d\n", value1); // Expected: 30

    // Test Case 2: Valid index at the beginning
    int index2 = 0;
    printf("\nTest Case 2: index = %d\n", index2);
    int value2 = getValueFromArray(data, size, index2);
    printf("Value returned: %d\n", value2); // Expected: 10

    // Test Case 3: Invalid negative index
    int index3 = -1;
    printf("\nTest Case 3: index = %d\n", index3);
    int value3 = getValueFromArray(data, size, index3);
    printf("Value returned: %d\n", value3); // Expected: -1

    // Test Case 4: Invalid index equal to size
    int index4 = size;
    printf("\nTest Case 4: index = %d\n", index4);
    int value4 = getValueFromArray(data, size, index4);
    printf("Value returned: %d\n", value4); // Expected: -1

    // Test Case 5: Valid index at the end
    int index5 = 4;
    printf("\nTest Case 5: index = %d\n", index5);
    int value5 = getValueFromArray(data, size, index5);
    printf("Value returned: %d\n", value5); // Expected: 50

    return 0;
}