#include <iostream>
#include <vector>
#include <stdexcept>
#include <string>

/**
 * @brief Securely gets a value from a vector at a given index.
 * 
 * Performs bounds checking to prevent out-of-bounds access.
 * 
 * @param data The source vector of integers.
 * @param index The index of the element to retrieve.
 * @return The integer value at the specified index.
 * @throw std::out_of_range if the index is out of the vector's bounds.
 */
int getValueAtIndex(const std::vector<int>& data, int index) {
    // Explicitly check for both negative indices and indices >= size.
    if (index >= 0 && static_cast<size_t>(index) < data.size()) {
        return data.at(static_cast<size_t>(index));
    } else {
        throw std::out_of_range("Index " + std::to_string(index) + " is out of bounds for vector of size " + std::to_string(data.size()));
    }
}

int main() {
    std::vector<int> data = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    
    // 5 Test cases: valid middle, valid start, valid end, invalid negative, invalid positive
    int testIndexes[] = {5, 0, 9, -1, 10};

    std::cout << "Testing in C++..." << std::endl;
    std::cout << "Vector: {10, 20, 30, 40, 50, 60, 70, 80, 90, 100}" << std::endl;
    for (int index : testIndexes) {
        try {
            int value = getValueAtIndex(data, index);
            std::cout << "Test case index " << index << ": Success! Value is " << value << std::endl;
        } catch (const std::out_of_range& e) {
            std::cout << "Test case index " << index << ": Error! " << e.what() << std::endl;
        }
    }
    return 0;
}