#include <iostream>
#include <vector>

// Define the sequence globally
const std::vector<int> id_sequence = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

/**
 * @brief Returns the value at the given index in the id_sequence vector.
 * 
 * @param index The index of the value to retrieve.
 * @return The value at the specified index, or -1 if the index is out of bounds.
 */
int getValueAtIndex(int index) {
    if (index >= 0 && static_cast<size_t>(index) < id_sequence.size()) {
        return id_sequence[index];
    } else {
        return -1; // Indicate index out of bounds
    }
}

int main() {
    // 5 test cases
    int testIndices[] = {0, 3, 9, -1, 10};

    std::cout << "Testing getValueAtIndex function:" << std::endl;
    for (int index : testIndices) {
        int value = getValueAtIndex(index);
        std::cout << "Value at index " << index << ": " << value << std::endl;
    }

    return 0;
}