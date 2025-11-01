#include <iostream>
#include <vector>
#include <stdexcept> // For std::out_of_range

/**
 * @brief Returns the value at the specified index in the vector.
 * 
 * Performs bounds checking to prevent security vulnerabilities like buffer over-reads.
 * 
 * @param arr The source vector.
 * @param index The index of the element to retrieve.
 * @return The integer value at the specified index.
 * @throw std::out_of_range if the index is out of the vector's bounds.
 */
int getValueAtIndex(const std::vector<int>& arr, int index) {
    // Security: Check if the index is within the valid bounds of the vector.
    // Using vector::at(index) would also perform this check and throw std::out_of_range.
    // This explicit check makes the security measure clear.
    if (index < 0 || index >= arr.size()) {
        throw std::out_of_range("Index is out of bounds");
    }
    return arr[index];
}

int main() {
    std::vector<int> data = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    
    // 5 Test Cases
    int testIndexes[] = {3, 0, 9, -1, 10};

    for (int index : testIndexes) {
        try {
            int value = getValueAtIndex(data, index);
            std::cout << "Test Case - Index: " << index << std::endl;
            std::cout << "Value at index " << index << " is: " << value << std::endl;
        } catch (const std::out_of_range& e) {
            std::cout << "Test Case - Index: " << index << std::endl;
            std::cout << "Error: " << e.what() << std::endl;
        }
        std::cout << "--------------------" << std::endl;
    }
    
    return 0;
}