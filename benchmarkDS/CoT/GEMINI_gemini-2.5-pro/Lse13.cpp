#include <iostream>
#include <vector>
#include <stdexcept>

// A predefined sequence of IDs using a std::vector for safety and flexibility.
const std::vector<int> id_sequence = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

/**
 * @brief Retrieves the value from id_sequence at a specified index.
 * 
 * @param index The index of the value to retrieve.
 * @return The integer value at the given index.
 * @throw std::out_of_range if the index is out of the valid range.
 */
int getValueAtIndex(int index) {
    // Security: Check if the index is within the valid bounds of the vector.
    // The .at() method provides this check automatically, but we do it manually
    // for clarity and to throw a custom message if desired.
    if (index >= 0 && index < id_sequence.size()) {
        return id_sequence[index]; // or id_sequence.at(index)
    } else {
        // If the index is invalid, throw a standard exception.
        throw std::out_of_range("Error: Index " + std::to_string(index) + " is out of bounds.");
    }
}

int main() {
    // 5 test cases for the getValueAtIndex function.
    int testIndexes[] = {0, 4, 9, -1, 10};

    std::cout << "--- Running CPP Test Cases ---" << std::endl;
    for (int index : testIndexes) {
        try {
            int value = getValueAtIndex(index);
            std::cout << "Value at index " << index << ": " << value << std::endl;
        } catch (const std::out_of_range& e) {
            std::cout << "Attempted to access index " << index << ". " << e.what() << std::endl;
        }
    }
    return 0;
}