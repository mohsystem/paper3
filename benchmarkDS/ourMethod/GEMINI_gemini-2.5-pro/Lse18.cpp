#include <iostream>
#include <vector>
#include <stdexcept>

// A predefined vector of integers
const std::vector<int> DATA = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

/**
 * @brief Returns the value from a predefined vector at the given index.
 * 
 * @param index The index of the element to retrieve.
 * @return int The value at the specified index.
 * @throw std::out_of_range if the index is out of bounds.
 */
int getValueAtIndex(int index) {
    // Rule #1, #4: Validate the index to ensure it is within the vector's boundaries.
    // Using vector::size() for a safe size check.
    if (index < 0 || static_cast<size_t>(index) >= DATA.size()) {
        // Rule #8: Fail closed by throwing a standard exception.
        throw std::out_of_range("Index is out of range.");
    }
    // Using operator[] for performance after the bounds check is complete.
    // Alternatively, .at(index) could be used, which performs its own check.
    return DATA[index];
}

int main() {
    // 5 test cases
    int testIndexes[] = {0, 5, 9, -1, 10};

    for (int index : testIndexes) {
        // Rule #7: Ensure all exceptions are caught and handled appropriately.
        try {
            int value = getValueAtIndex(index);
            std::cout << "Value at index " << index << ": " << value << std::endl;
        } catch (const std::out_of_range& e) {
            std::cerr << "Error for index " << index << ": " << e.what() << std::endl;
        }
    }

    return 0;
}