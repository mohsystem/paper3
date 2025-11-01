#include <iostream>
#include <vector>
#include <stdexcept>

// The predefined sequence of IDs.
const std::vector<int> id_sequence = {101, 102, 103, 104, 105, 106, 107, 108, 109, 110};

/**
 * Returns the value at the specified index in the id_sequence vector.
 *
 * @param index The index of the element to return.
 * @return The integer value at the specified index.
 * @throws std::out_of_range if the index is out of range.
 */
int getValueAtIndex(int index) {
    // Rule #1 & #3: Validate input and check boundaries.
    // std::vector::at() performs bounds checking and throws std::out_of_range on failure.
    return id_sequence.at(index);
}

int main() {
    // 5 test cases
    int testIndices[] = {0, 9, 5, -1, 10};

    for (int index : testIndices) {
        std::cout << "Testing with index: " << index << std::endl;
        try {
            // Rule #5: Ensure that all exceptions are caught and handled appropriately.
            int value = getValueAtIndex(index);
            std::cout << "Value at index " << index << ": " << value << std::endl;
        } catch (const std::out_of_range& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
        std::cout << "--------------------" << std::endl;
    }

    return 0;
}