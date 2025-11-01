#include <stdio.h>

/**
 * @brief Safely gets the value at a specified index in an array.
 * 
 * Performs bounds checking to prevent security vulnerabilities like buffer over-reads.
 * This function returns a status code and provides the result via a pointer,
 * which is a common and safe pattern in C for functions that can fail.
 * 
 * @param arr The source array.
 * @param size The number of elements in the array.
 * @param index The index of the element to retrieve.
 * @param result A pointer to an integer where the result will be stored on success.
 * @return 0 on success, -1 on failure (e.g., index out of bounds).
 */
int getValueAtIndex(const int arr[], int size, int index, int* result) {
    // Security: Check if the array pointer is valid and if the index is within bounds.
    if (arr == NULL || result == NULL) {
        return -1; // Indicate error due to null pointers
    }
    if (index < 0 || index >= size) {
        return -1; // Indicate error due to out-of-bounds index
    }
    
    // If checks pass, assign the value to the result pointer and indicate success.
    *result = arr[index];
    return 0; // Indicate success
}

int main() {
    int data[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    int size = sizeof(data) / sizeof(data[0]);
    
    // 5 Test Cases
    int testIndexes[] = {3, 0, 9, -1, 10};
    int numTestCases = sizeof(testIndexes) / sizeof(testIndexes[0]);

    for (int i = 0; i < numTestCases; ++i) {
        int index = testIndexes[i];
        int value;
        
        printf("Test Case - Index: %d\n", index);
        
        // Check the return status of the function
        if (getValueAtIndex(data, size, index, &value) == 0) {
            printf("Value at index %d is: %d\n", index, value);
        } else {
            printf("Error: Index %d is out of bounds for array of size %d\n", index, size);
        }
        printf("--------------------\n");
    }
    
    return 0;
}