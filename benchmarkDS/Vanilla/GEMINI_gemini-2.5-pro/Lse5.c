#include <stdio.h>
#include <stdlib.h> // For atoi

/**
 * Accepts an array, its size, and an index, then prints the element at that index.
 * It includes error handling for out-of-bounds indices.
 * @param data The integer array to access.
 * @param size The number of elements in the array.
 * @param index The index of the element to print.
 */
void getElementAtIndex(int data[], int size, int index) {
    if (index >= 0 && index < size) {
        printf("Element at index %d: %d\n", index, data[index]);
    } else {
        printf("Error: Index %d is out of bounds for an array of size %d.\n", index, size);
    }
}

int main(int argc, char* argv[]) {
    int elements[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    int size = sizeof(elements) / sizeof(elements[0]);

    // Check if a command-line argument is provided
    if (argc > 1) {
        // atoi returns 0 on error, which could be a valid index.
        // For this simple case, we assume valid integer input.
        int indexFromArg = atoi(argv[1]);
        printf("--- Running with command-line argument ---\n");
        getElementAtIndex(elements, size, indexFromArg);
    } else {
        // If no command-line arguments, run the 5 built-in test cases
        printf("--- Running built-in test cases ---\n");
        
        // Test Case 1: Accessing the first element
        getElementAtIndex(elements, size, 0);
        
        // Test Case 2: Accessing an element in the middle
        getElementAtIndex(elements, size, 4);

        // Test Case 3: Accessing the last element
        getElementAtIndex(elements, size, 9);
        
        // Test Case 4: Accessing with a negative index (out of bounds)
        getElementAtIndex(elements, size, -1);
        
        // Test Case 5: Accessing an index equal to the array size (out of bounds)
        getElementAtIndex(elements, size, 10);
    }

    return 0;
}