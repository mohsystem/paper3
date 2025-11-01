#include <iostream>
#include <vector>

/**
 * Returns the value at a given index in an array.
 * Performs bounds checking to prevent out-of-bounds access.
 *
 * @param arr   The array of integers.
 * @param size  The size of the array.
 * @param index The index of the element to retrieve.
 * @return The value at the specified index, or -1 if the index is out of bounds.
 */
int getValueFromArray(const int arr[], int size, int index) {
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

    std::cout << "Enter an integer index:" << std::endl;
    
    // Security: Check if the input stream is valid and the value is an integer.
    if (!(std::cin >> index)) {
        std::cerr << "Error: Invalid input. Please enter an integer." << std::endl;
        return 1; // Exit with a general error code for bad input.
    }

    int result = getValueFromArray(id_sequence, size, index);

    if (result == -1) {
        std::cerr << "Error: Index out of bounds." << std::endl;
        return 1; // Exit with a general error code for out-of-bounds access.
    } else {
        // The value returned from main is the program's exit code.
        // Note: Exit codes are typically 0-255. Larger values may be truncated.
        std::cout << "Value at index " << index << " is " << result << ". Exiting with this value." << std::endl;
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