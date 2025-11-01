#include <iostream>
#include <vector>

// The predefined array (using a vector for safety and flexibility)
const std::vector<int> anArray = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

/**
 * Returns the value at the specified index of the predefined array.
 *
 * @param index The index of the element to return.
 * @return The element at the specified index, or -1 if the index is out of bounds.
 */
int getValueAtIndex(int index) {
    // Check if the index is within the valid range
    if (index >= 0 && index < anArray.size()) {
        return anArray[index];
    } else {
        // Return a special value to indicate an invalid index
        return -1;
    }
}

int main() {
    // 5 Test Cases
    std::cout << "Test Case 1 (index = 3): " << getValueAtIndex(3) << std::endl;
    std::cout << "Test Case 2 (index = 0): " << getValueAtIndex(0) << std::endl;
    std::cout << "Test Case 3 (index = 9): " << getValueAtIndex(9) << std::endl;
    std::cout << "Test Case 4 (index = 10): " << getValueAtIndex(10) << std::endl; // Out of bounds
    std::cout << "Test Case 5 (index = -1): " << getValueAtIndex(-1) << std::endl; // Out of bounds
    
    return 0;
}