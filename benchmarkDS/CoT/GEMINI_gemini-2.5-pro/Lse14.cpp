#include <iostream>
#include <vector>
#include <stdexcept>

// A predefined constant vector of IDs.
const std::vector<int> id_sequence = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

/**
 * Retrieves the value from id_sequence at a specified index.
 *
 * @param index The index of the element to retrieve.
 * @return The value at the specified index.
 * @throws std::out_of_range if the index is out of the valid range.
 */
int getValueAtIndex(int index) {
    // Security Check: Validate that the index is within the bounds of the vector.
    // Using .at(index) would also work as it performs bounds checking automatically.
    if (index >= 0 && index < id_sequence.size()) {
        return id_sequence[index];
    } else {
        // If the index is invalid, throw an exception to indicate the error.
        throw std::out_of_range("Error: Index " + std::to_string(index) + " is out of bounds.");
    }
}

int main() {
    // 5 Test Cases
    int testIndexes[] = {3, 0, 9, -1, 10};

    for (int i = 0; i < 5; ++i) {
        int indexToTest = testIndexes[i];
        std::cout << "Test Case " << (i + 1) << ": Testing index " << indexToTest << std::endl;
        try {
            int value = getValueAtIndex(indexToTest);
            std::cout << " -> Success: Value at index " << indexToTest << " is " << value << std::endl;
        } catch (const std::out_of_range& e) {
            std::cerr << " -> " << e.what() << std::endl;
        }
        std::cout << "--------------------" << std::endl;
    }

    return 0;
}