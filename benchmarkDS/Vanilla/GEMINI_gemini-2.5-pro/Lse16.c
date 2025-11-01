#include <stdio.h>

/**
 * Returns the value at a given index in an integer array.
 * @param arr The input array.
 * @param size The number of elements in the array.
 * @param index The index of the element to retrieve.
 * @return The value at the specified index, or -1 if the index is out of bounds.
 */
int getValueAtIndex(int arr[], int size, int index) {
    if (index >= 0 && index < size) {
        return arr[index];
    } else {
        // Return a sentinel value to indicate the index is out of bounds
        return -1;
    }
}

int main() {
    int data[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    int size = sizeof(data) / sizeof(data[0]);

    // 5 test cases
    printf("Test Case 1 (index 0): %d\n", getValueAtIndex(data, size, 0));
    printf("Test Case 2 (index 5): %d\n", getValueAtIndex(data, size, 5));
    printf("Test Case 3 (index 9): %d\n", getValueAtIndex(data, size, 9));
    printf("Test Case 4 (index -1): %d\n", getValueAtIndex(data, size, -1));
    printf("Test Case 5 (index 10): %d\n", getValueAtIndex(data, size, 10));

    return 0;
}