#include <iostream>
#include <vector>
#include <stdexcept>
#include <string>

// The predefined vector of IDs. Using std::vector is safer and more modern C++.
const std::vector<int> id_sequence = {101, 102, 103, 104, 105, 106, 107, 108, 109, 110};

/**
 * @brief Retrieves the value from id_sequence at a specified index.
 * This function is secure against index out-of-bounds errors.
 *
 * @param index The index of the value to retrieve.
 * @return The value at the specified index.
 * @throw std::out_of_range if the index is out of the valid range.
 */
int getValueAtIndex(int index) {
    // Secure: Check if the index is within the valid bounds of the vector.
    // Using .at() method provides automatic bounds checking.
    // Alternatively, a manual check:
    if (index < 0 || index >= id_sequence.size()) {
        throw std::out_of_range("Error: Index " + std::to_string(index) + " is out of bounds for the vector of size " + std::to_string(id_sequence.size()) + ".");
    }
    return id_sequence[index];
}

/**
 * @brief Main function with 5 test cases to demonstrate functionality.
 */
int main() {
    // Test cases: valid start, valid middle, valid end, invalid negative, invalid positive
    int testCases[] = {0, 4, 9, -1, 10};

    std::cout << "--- Running C++ Test Cases ---" << std::endl;
    for (int index : testCases) {
        try {
            int value = getValueAtIndex(index);
            std::cout << "SUCCESS: Value at index " << index << " is " << value << std::endl;
        } catch (const std::out_of_range& e) {
            std::cout << "FAILURE: Attempted to access index " << index << ". " << e.what() << std::endl;
        }
    }
    return 0;
}