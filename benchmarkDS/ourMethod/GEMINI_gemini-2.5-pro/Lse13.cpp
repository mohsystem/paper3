#include <iostream>
#include <vector>
#include <stdexcept>

// A predefined, constant sequence of IDs.
const std::vector<int> id_sequence = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

/**
 * @brief Returns the value at the specified index in the id_sequence vector.
 * 
 * @param index The index of the element to retrieve.
 * @return The value at the specified index.
 * @throw std::out_of_range if the index is out of bounds.
 */
int getValueAtIndex(int index) {
    // Rule #1, #4: Using vector::at() provides automatic bounds checking.
    // std::vector::at performs bounds checking and throws std::out_of_range on failure.
    // We check for negative index explicitly as size_type is unsigned.
    if (index < 0) {
         throw std::out_of_range("Error: Index cannot be negative.");
    }
    // The cast to size_t is safe here because we've checked for negative values.
    return id_sequence.at(static_cast<size_t>(index));
}

/**
 * @brief Main function to run test cases.
 * 
 * @return int Exit code. 0 on success.
 */
int main() {
    // 5 test cases for various scenarios.
    int testIndices[] = {0, 5, 9, -1, 10};

    std::cout << "Running 5 test cases..." << std::endl;
    for (int i = 0; i < 5; ++i) {
        int testIndex = testIndices[i];
        std::cout << "\n--- Test Case " << (i + 1) << " ---" << std::endl;
        std::cout << "Attempting to access index: " << testIndex << std::endl;
        try {
            // Rule #7: Ensure that all exceptions are caught and handled appropriately.
            int value = getValueAtIndex(testIndex);
            std::cout << "Value at index " << testIndex << ": " << value << std::endl;
        } catch (const std::out_of_range& e) {
            std::cerr << e.what() << std::endl;
        }
    }

    return 0;
}