
#include <iostream>
#include <stdexcept>
#include <vector>

int getValueAtIndex(int index, const std::vector<int>& array) {
    if (index < 0 || index >= static_cast<int>(array.size())) {
        throw std::out_of_range("Index out of bounds");
    }
    return array[index];
}

int main() {
    std::vector<int> testArray = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    
    // Test case 1: Valid index at beginning
    try {
        std::cout << "Test 1 - Index 0: " << getValueAtIndex(0, testArray) << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 1 Error: " << e.what() << std::endl;
    }
    
    // Test case 2: Valid index in middle
    try {
        std::cout << "Test 2 - Index 5: " << getValueAtIndex(5, testArray) << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 2 Error: " << e.what() << std::endl;
    }
    
    // Test case 3: Valid index at end
    try {
        std::cout << "Test 3 - Index 9: " << getValueAtIndex(9, testArray) << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 3 Error: " << e.what() << std::endl;
    }
    
    // Test case 4: Invalid negative index
    try {
        std::cout << "Test 4 - Index -1: " << getValueAtIndex(-1, testArray) << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 4 Error: " << e.what() << std::endl;
    }
    
    // Test case 5: Invalid index beyond array length
    try {
        std::cout << "Test 5 - Index 15: " << getValueAtIndex(15, testArray) << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 5 Error: " << e.what() << std::endl;
    }
    
    return 0;
}
