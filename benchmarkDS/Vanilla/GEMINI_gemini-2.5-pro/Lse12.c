#include <stdio.h>

/**
 * Returns the value from an array at a given index.
 *
 * @param arr The array of integers.
 * @param size The size of the array.
 * @param index The index of the element to retrieve.
 * @return The integer value at the specified index.
 */
int getValueFromArray(int arr[], int size, int index) {
    // Assuming the index is always within the valid bounds of the array.
    return arr[index];
}

int main() {
    int sampleArray[] = {10, 20, 30, 40, 50};
    int arraySize = sizeof(sampleArray) / sizeof(sampleArray[0]);

    // 5 Test Cases
    printf("C Test Cases:\n");

    // Test Case 1
    printf("Value at index 0: %d\n", getValueFromArray(sampleArray, arraySize, 0));
    // Test Case 2
    printf("Value at index 2: %d\n", getValueFromArray(sampleArray, arraySize, 2));
    // Test Case 3
    printf("Value at index 4: %d\n", getValueFromArray(sampleArray, arraySize, 4));
    // Test Case 4
    printf("Value at index 1: %d\n", getValueFromArray(sampleArray, arraySize, 1));
    // Test Case 5
    printf("Value at index 3: %d\n", getValueFromArray(sampleArray, arraySize, 3));

    return 0;
}