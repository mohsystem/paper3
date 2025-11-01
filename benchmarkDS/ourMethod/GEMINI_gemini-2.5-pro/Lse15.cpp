#include <iostream>
#include <vector>
#include <stdexcept>
#include <string>
#include <limits>

// Define a constant sequence of IDs using a std::vector for safety.
const std::vector<int> ID_SEQUENCE = {101, 102, 103, 104, 105, 106, 107, 108, 109, 110};

/**
 * Returns the value at a given index in the ID_SEQUENCE vector.
 * This function demonstrates secure coding by validating the index
 * to prevent out-of-bounds access.
 *
 * @param index The index of the value to retrieve.
 * @return The integer value at the specified index.
 * @throws std::out_of_range if the index is out of bounds.
 */
int getValueAtIndex(int index) {
    // Rules #1, #2, #4: Check boundaries. Comparing a signed int with an
    // unsigned size() can be risky, so we check for negative first.
    if (index < 0 || static_cast<size_t>(index) >= ID_SEQUENCE.size()) {
        // Rule #5, #7: Throw an exception for error handling.
        throw std::out_of_range("Error: Index " + std::to_string(index) + " is out of bounds.");
    }
    // Safe to access with [] after bounds check.
    return ID_SEQUENCE[static_cast<size_t>(index)];
}

/**
 * A helper function to run a single test case and print the result.
 * @param testName The name of the test case.
 * @param index The index to test.
 */
void runTestCase(const std::string& testName, int index) {
    std::cout << testName << " with index " << index << ": ";
    try {
        int value = getValueAtIndex(index);
        std::cout << "Success! Value is " << value << std::endl;
    } catch (const std::out_of_range& e) {
        std::cerr << e.what() << std::endl;
    }
}

int main() {
    std::cout << "--- Running 5 Test Cases ---" << std::endl;
    runTestCase("Test Case 1", 0);
    runTestCase("Test Case 2", 5);
    runTestCase("Test Case 3", static_cast<int>(ID_SEQUENCE.size() - 1));
    runTestCase("Test Case 4", -1);
    runTestCase("Test Case 5", static_cast<int>(ID_SEQUENCE.size()));

    // The prompt "The function first prompts the user for an index value" is interpreted
    // as the main application logic prompting the user before calling the function.
    std::cout << "\n--- Interactive User Test ---" << std::endl;
    std::cout << "Please enter an index value: ";
    int userIndex;
    std::cin >> userIndex;

    // Rule #4: Validate that the input stream is in a good state.
    if (std::cin.good()) {
        std::cout << "Testing with user index " << userIndex << ": ";
        try {
            int value = getValueAtIndex(userIndex);
            std::cout << "Success! Value is " << value << std::endl;
        } catch (const std::out_of_range& e) {
            std::cerr << e.what() << std::endl;
        }
    } else {
        std::cerr << "Error: Invalid input. Please enter a valid integer." << std::endl;
        // Clear error flags and discard remaining input in the buffer.
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    return 0;
}