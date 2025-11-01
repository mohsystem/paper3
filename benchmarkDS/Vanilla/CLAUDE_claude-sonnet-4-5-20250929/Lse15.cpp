
#include <iostream>
#include <stdexcept>

const int id_sequence[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
const int id_sequence_size = 10;

int getValueAtIndex(int index) {
    if (index >= 0 && index < id_sequence_size) {
        return id_sequence[index];
    }
    throw std::out_of_range("Index out of bounds");
}

int main() {
    // Test case 1
    try {
        std::cout << "Test 1 - Index 0: " << getValueAtIndex(0) << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 1 - Error: " << e.what() << std::endl;
    }
    
    // Test case 2
    try {
        std::cout << "Test 2 - Index 5: " << getValueAtIndex(5) << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 2 - Error: " << e.what() << std::endl;
    }
    
    // Test case 3
    try {
        std::cout << "Test 3 - Index 9: " << getValueAtIndex(9) << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 3 - Error: " << e.what() << std::endl;
    }
    
    // Test case 4
    try {
        std::cout << "Test 4 - Index -1: " << getValueAtIndex(-1) << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 4 - Error: " << e.what() << std::endl;
    }
    
    // Test case 5
    try {
        std::cout << "Test 5 - Index 15: " << getValueAtIndex(15) << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 5 - Error: " << e.what() << std::endl;
    }
    
    return 0;
}
