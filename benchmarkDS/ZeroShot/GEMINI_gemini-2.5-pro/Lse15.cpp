#include <iostream>
#include <vector>
#include <stdexcept>
#include <string>

// Using a static const vector for the sequence. It's safer as it knows its own size.
static const std::vector<int> id_sequence = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

/**
 * Securely gets a value from the id_sequence vector at a given index.
 * It performs bounds checking to prevent out-of-bounds access.
 *
 * @param index The index of the element to retrieve.
 * @return The value at the specified index.
 * @throws std::out_of_range if the index is out of bounds.
 */
int getValueAtIndex(int index) {
    // Secure: The .at() method of std::vector provides automatic bounds checking
    // and throws std::out_of_range if the index is invalid.
    try {
        return id_sequence.at(index);
    } catch (const std::out_of_range& oor) {
        // Re-throw with a more descriptive message.
        throw std::out_of_range("Error: Index " + std::to_string(index) + " is out of bounds. Must be between 0 and " + std::to_string(id_sequence.size() - 1) + ".");
    }
}

int main() {
    // 5 test cases for the getValueAtIndex function.
    int testIndexes[] = {0, 5, 9, -1, 10};

    std::cout << "Running 5 test cases..." << std::endl;

    for (int i = 0; i < 5; ++i) {
        int index = testIndexes[i];
        std::cout << "\n--- Test Case " << (i + 1) << ": Trying index " << index << " ---" << std::endl;
        try {
            int value = getValueAtIndex(index);
            std::cout << "Success: Value at index " << index << " is " << value << std::endl;
        } catch (const std::out_of_range& e) {
            std::cerr << e.what() << std::endl;
        }
    }

    return 0;
}