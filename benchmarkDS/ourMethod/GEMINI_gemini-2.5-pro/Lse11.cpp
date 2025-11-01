#include <iostream>
#include <vector>
#include <stdexcept>
#include <limits>

/**
 * @brief Returns the value from the vector at the given index using bounds-checked access.
 * 
 * @param data The vector of integers.
 * @param index The index of the element to retrieve.
 * @return The value at the given index.
 * @throws std::out_of_range if the index is out of bounds.
 */
int getValueFromArray(const std::vector<int>& data, int index) {
    // Rule #1: The .at() method performs a boundary check and throws std::out_of_range on failure.
    // The cast to size_t is safe because negative indices are handled by the logic of .at()
    // which compares against a potentially large unsigned size.
    return data.at(static_cast<size_t>(index));
}

/**
 * @brief Runs a single test case and prints the result.
 */
void runTest(const std::vector<int>& data, int index, int expectedExitCode) {
    std::cout << "Test with index " << index << ": ";
    int exitCode;
    try {
        exitCode = getValueFromArray(data, index);
    } catch (const std::out_of_range&) {
        exitCode = 1; // Simulate error exit code
    }

    if (exitCode == expectedExitCode) {
        std::cout << "PASSED (Expected exit code: " << expectedExitCode << ", Got: " << exitCode << ")" << std::endl;
    } else {
        std::cout << "FAILED (Expected exit code: " << expectedExitCode << ", Got: " << exitCode << ")" << std::endl;
    }
}

/**
 * @brief Main function to read an index and return an array value as the exit code.
 */
int main() {
    const std::vector<int> id_sequence = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

    // --- Test Cases ---
    std::cout << "--- Running Test Cases ---" << std::endl;
    runTest(id_sequence, 0, 10);    // Test case 1: Valid, first element
    runTest(id_sequence, 9, 100);   // Test case 2: Valid, last element
    runTest(id_sequence, -1, 1);    // Test case 3: Invalid, negative index
    runTest(id_sequence, 10, 1);    // Test case 4: Invalid, out of bounds
    // Test case 5 (invalid format) is handled by the interactive part
    std::cout << "--- Test Cases Finished ---" << std::endl;
    
    std::cout << "\nEnter an integer index:" << std::endl;
    
    int index;
    // Rule #4: Ensure all input is validated.
    std::cin >> index;

    if (std::cin.fail()) {
        std::cerr << "Error: Invalid input. Please enter an integer." << std::endl;
        return 1;
    }

    // Rule #7: Ensure that all exceptions are caught and handled.
    try {
        int value = getValueFromArray(id_sequence, index);
        std::cout << "Value at index " << index << " is " << value << ". Exiting with this value." << std::endl;
        return value;
    } catch (const std::out_of_range& e) {
        std::cerr << "Error: Index out of bounds." << std::endl;
        return 1;
    }
}