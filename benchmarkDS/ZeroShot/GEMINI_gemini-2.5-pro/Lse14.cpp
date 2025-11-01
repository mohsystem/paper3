#include <iostream>
#include <vector>
#include <stdexcept> // For std::out_of_range

// The sequence of IDs, defined as a constant vector.
const std::vector<int> id_sequence = {101, 102, 103, 104, 105, 106, 107, 108, 109, 110};

/**
 * Retrieves the value from id_sequence at a specified index.
 * This function is secure because it uses the .at() method, which
 * provides automatic bounds checking and throws an exception on failure.
 *
 * @param index The index of the element to retrieve.
 * @return The value at the specified index.
 * @throws std::out_of_range if the index is out of bounds.
 */
int getValueAtIndex(int index) {
    // The .at() method automatically checks if the index is within the valid range.
    // If the index is out of bounds, it throws a std::out_of_range exception.
    // This is more secure than using the [] operator which results in undefined behavior.
    return id_sequence.at(index);
}

int main() {
    // 5 test cases
    int testIndexes[] = {0, 9, 4, -1, 10};

    std::cout << "Running 5 test cases..." << std::endl;
    for (int i = 0; i < 5; ++i) {
        int index = testIndexes[i];
        std::cout << "\n--- Test Case " << (i + 1) << " ---" << std::endl;
        std::cout << "Attempting to get value at index: " << index << std::endl;
        try {
            int value = getValueAtIndex(index);
            std::cout << "Success! Value at index " << index << " is: " << value << std::endl;
        } catch (const std::out_of_range& e) {
            std::cerr << "Error: Index " << index << " is out of bounds." << std::endl;
        }
    }

    return 0;
}