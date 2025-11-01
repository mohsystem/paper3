#include <iostream>
#include <vector>
#include <stdexcept>

// The predefined vector
const std::vector<int> id_sequence = {101, 102, 103, 104, 105, 106, 107, 108, 109, 110};

/**
 * Returns the value at the given index in the id_sequence vector.
 * @param index The index of the value to retrieve.
 * @return The integer value at the specified index.
 * @throws std::out_of_range if the index is invalid.
 */
int getValueAtIndex(int index) {
    // The prompt for user input is handled in main for testing purposes.
    // This function directly takes the index as a parameter.
    // std::vector::at() provides bounds checking and throws std::out_of_range on error.
    return id_sequence.at(index);
}

int main() {
    // 5 test cases
    int testIndices[] = {0, 4, 9, -1, 10};

    for (int i = 0; i < 5; ++i) {
        int index = testIndices[i];
        std::cout << "Test Case " << (i + 1) << ": Getting value at index " << index << std::endl;
        try {
            int value = getValueAtIndex(index);
            std::cout << "Value: " << value << std::endl;
        } catch (const std::out_of_range& e) {
            std::cerr << "Error: Index " << index << " is out of bounds." << std::endl;
        }
        std::cout << "--------------------" << std::endl;
    }

    return 0;
}