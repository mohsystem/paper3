#include <stdio.h>

/**
 * Returns the value at a given index in an array.
 * Performs bounds checking to prevent out-of-bounds access.
 *
 * @param arr   The array of integers.
 * @param size  The size of the array.
 * @param index The index of the element to retrieve.
 * @return The value at the specified index, or -1 if the index is out of bounds.
 */
int getValueFromArray(const int* arr, int size, int index) {
    // Security: Check if the index is within the valid bounds of the array.
    if (index >= 0 && index < size) {
        return arr[index];
    }
    // Return an error code for invalid index.
    return -1;
}

int main() {
    const int id_sequence[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    const int size = sizeof(id_sequence) / sizeof(id_sequence[0]);
    int index;

    printf("Enter an integer index:\n");

    // Security: Check the return value of scanf to ensure an integer was read.
    if (scanf("%d", &index) != 1) {
        fprintf(stderr, "Error: Invalid input. Please enter an integer.\n");
        return 1; // Exit with a general error code for bad input.
    }

    int result = getValueFromArray(id_sequence, size, index);

    if (result == -1) {
        fprintf(stderr, "Error: Index out of bounds.\n");
        return 1; // Exit with a general error code for out-of-bounds access.
    } else {
        // The value returned from main is the program's exit code.
        // Note: Exit codes are typically 0-255. Larger values may be truncated.
        printf("Value at index %d is %d. Exiting with this value.\n", index, result);
        return result;
    }
}
/*
Main method test cases:
1. Input: 2
   Expected Output: Program exits with code 30. (Value at index 2 is 30)
2. Input: 0
   Expected Output: Program exits with code 10. (Value at index 0 is 10)
3. Input: 9
   Expected Output: Program exits with code 100. (Value at index 9 is 100)
4. Input: -1
   Expected Output: Prints "Error: Index out of bounds." and exits with code 1.
5. Input: 10
   Expected Output: Prints "Error: Index out of bounds." and exits with code 1.
*/