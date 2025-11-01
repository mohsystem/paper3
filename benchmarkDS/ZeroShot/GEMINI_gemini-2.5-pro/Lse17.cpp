#include <iostream>
#include <vector>
#include <stdexcept>

/**
 * Securely retrieves the value at a given index from a vector.
 * This function uses the .at() method, which provides automatic bounds checking.
 *
 * @param data The vector of integers.
 * @param index The index of the element to retrieve.
 * @return The integer value at the specified index.
 * @throws std::out_of_range if the index is out of the valid range.
 */
int getValueAtIndex(const std::vector<int>& data, int index) {
    // Secure: Use the .at() method which performs bounds checking automatically
    // and throws std::out_of_range on failure.
    return data.at(index);
}

int main() {
    const std::vector<int> my_data = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    
    // 5 Test Cases: 3 valid, 2 invalid
    int testIndexes[] = {3, 0, 9, -1, 10};

    std::cout << "--- C++: Testing getValueAtIndex ---" << std::endl;
    for (int index : testIndexes) {
        try {
            int value = getValueAtIndex(my_data, index);
            std::cout << "Test Case: index = " << index << std::endl;
            std::cout << "  -> Success: Value is " << value << std::endl;
        } catch (const std::out_of_range& e) {
            std::cout << "Test Case: index = " << index << std::endl;
            std::cout << "  -> Error: Index is out of bounds." << std::endl;
        }
        std::cout << "------------------------------------" << std::endl;
    }
    return 0;
}