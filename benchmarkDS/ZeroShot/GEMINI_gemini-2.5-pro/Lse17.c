#include <stdio.h>
#include <stdbool.h>

/**
 * Securely retrieves the value at a given index from an array.
 * This function checks for bounds before accessing the array element.
 *
 * @param data Pointer to the integer array.
 * @param size The total number of elements in the array.
 * @param index The index of the element to retrieve.
 * @param out_value A pointer to an integer where the result will be stored on success.
 * @return true if the index is valid and the value was retrieved, false otherwise.
 */
bool getValueAtIndex(const int* data, int size, int index, int* out_value) {
    // Secure: Check if the index is within the valid bounds of the array.
    if (out_value != NULL && index >= 0 && index < size) {
        *out_value = data[index];
        return true;
    } else {
        // Invalid index or null output pointer, return failure.
        return false;
    }
}

int main() {
    const int data[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    const int data_size = sizeof(data) / sizeof(data[0]);
    
    // 5 Test Cases: 3 valid, 2 invalid
    int testIndexes[] = {3, 0, 9, -1, 10};
    int num_tests = sizeof(testIndexes) / sizeof(testIndexes[0]);

    printf("--- C: Testing getValueAtIndex ---\n");
    for (int i = 0; i < num_tests; ++i) {
        int index = testIndexes[i];
        int value;
        
        printf("Test Case: index = %d\n", index);
        if (getValueAtIndex(data, data_size, index, &value)) {
            printf("  -> Success: Value is %d\n", value);
        } else {
            printf("  -> Error: Index is out of bounds.\n");
        }
        printf("--------------------------------\n");
    }

    return 0;
}