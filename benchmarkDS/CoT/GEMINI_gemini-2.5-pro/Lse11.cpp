#include <iostream>

/**
 * @brief Gets a value from an array at a specific index with bounds checking.
 * The function prevents out-of-bounds access by validating the index.
 *
 * @param arr Pointer to the constant integer array.
 * @param size The size of the array.
 * @param index The index of the element to retrieve.
 * @return The value at the given index, or 1 as an error code if the index is out of bounds.
 */
int getValueFromArray(const int arr[], int size, int index) {
    // Security: Check if the index is within the valid bounds of the array (0 to size-1).
    // This prevents reading from arbitrary memory locations.
    if (index < 0 || index >= size) {
        std::cerr << "Error: Index " << index << " is out of bounds for array size " << size << "." << std::endl;
        return 1; // Return an error code for out-of-bounds access.
    }
    return arr[index];
}

/**
 * @brief Main function to demonstrate getValueFromArray with 5 test cases.
 * The original prompt required reading a single value and exiting with the result code.
 * To show 5 test cases in a single run, this main function simulates those runs
 * and prints what the result/exit code would be.
 *
 * @return 0 on successful execution of tests.
 */
int main() {
    const int id_sequence[] = {10, 20, 30, 40, 50};
    int size = sizeof(id_sequence) / sizeof(id_sequence[0]);

    // 5 Test Cases to demonstrate the functionality and security checks.
    int testIndices[] = {
        0,  // Test Case 1: Valid index at the beginning
        2,  // Test Case 2: Valid index in the middle
        4,  // Test Case 3: Valid index at the end
        -1, // Test Case 4: Invalid index (negative)
        5   // Test Case 5: Invalid index (out of bounds high)
    };

    std::cout << "Running 5 test cases for id_sequence: {10, 20, 30, 40, 50}" << std::endl;
    std::cout << "------------------------------------------" << std::endl;

    for (int i = 0; i < 5; ++i) {
        int index = testIndices[i];
        std::cout << "Test Case " << (i + 1) << ": Requesting index " << index << std::endl;
        
        int result = getValueFromArray(id_sequence, size, index);
        
        std::cout << " -> Function returned: " << result << std::endl;
        if (result == 1 && (index < 0 || index >= size)) {
            std::cout << "   (This would be an exit code indicating an error)" << std::endl;
        } else {
            std::cout << "   (Value at index " << index << " is " << result << ". This would be the exit code)" << std::endl;
        }
        std::cout << "------------------------------------------" << std::endl;
    }

    return 0; // Main program exits successfully after tests.
}