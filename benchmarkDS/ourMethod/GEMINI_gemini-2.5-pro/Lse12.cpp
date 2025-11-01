#include <iostream>
#include <vector>
#include <cstdlib> // For EXIT_SUCCESS, EXIT_FAILURE

/**
 * @brief Returns the value at a given index in an array.
 *
 * This function performs bounds checking. If the index is out of bounds,
 * it prints an error message to stderr and returns EXIT_FAILURE.
 *
 * @param arr The array of integers.
 * @param size The size of the array.
 * @param index The index of the element to retrieve.
 * @return The value at the specified index on success, or EXIT_FAILURE on error.
 */
int getValueFromArray(const int arr[], int size, int index) {
    // Rule #1, #4: Validate inputs and check boundaries
    if (index < 0 || index >= size) {
        std::cerr << "Error: Index " << index << " is out of bounds for array of size " << size << "." << std::endl;
        return EXIT_FAILURE;
    }
    return arr[index];
}

/**
 * @brief Helper function to run a single test case.
 */
void runTestCase(const int arr[], int size, int index, int expected) {
    std::cout << "Test case: index=" << index << ". ";
    int result = getValueFromArray(arr, size, index);
    if (result == expected) {
        std::cout << "Result: " << result << ". PASS" << std::endl;
    } else {
        std::cout << "Result: " << result << ". FAIL (Expected: " << expected << ")" << std::endl;
    }
}

int main() {
    const int data[] = {10, 20, 30, 40, 50};
    const int dataSize = sizeof(data) / sizeof(data[0]);

    // --- Test Cases ---
    std::cout << "--- Running Test Cases ---" << std::endl;
    runTestCase(data, dataSize, 0, 10);
    runTestCase(data, dataSize, 4, 50);
    runTestCase(data, dataSize, 2, 30);
    runTestCase(data, dataSize, -1, EXIT_FAILURE); // Expect failure
    runTestCase(data, dataSize, 5, EXIT_FAILURE);  // Expect failure
    std::cout << "--- Test Cases Finished ---" << std::endl << std::endl;

    std::cout << "Enter an index to retrieve a value from the array:" << std::endl;
    int index;
    std::cin >> index;

    // Rule #4: Input validation
    if (std::cin.fail()) {
        std::cerr << "Error: Invalid input. Please enter an integer." << std::endl;
        return EXIT_FAILURE;
    }

    int result = getValueFromArray(data, dataSize, index);
    
    // Check if the call was successful before printing the result
    bool was_successful = !(result == EXIT_FAILURE && (index < 0 || index >= dataSize));
    if (was_successful) {
        std::cout << "Value at index " << index << " is " << result << std::endl;
    }

    // Use the return value as the exit code
    return result;
}