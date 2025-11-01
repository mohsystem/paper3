
#include <iostream>
#include <string>
#include <cstdlib>

std::string getElementAtIndex(int index) {
    std::string elements[] = {"apple", "banana", "cherry", "date", "elderberry"};
    int size = 5;
    
    if (index >= 0 && index < size) {
        return elements[index];
    } else {
        return "Index out of bounds";
    }
}

int main(int argc, char* argv[]) {
    // Test cases
    std::cout << "Test case 1: " << getElementAtIndex(0) << std::endl;
    std::cout << "Test case 2: " << getElementAtIndex(2) << std::endl;
    std::cout << "Test case 3: " << getElementAtIndex(4) << std::endl;
    std::cout << "Test case 4: " << getElementAtIndex(-1) << std::endl;
    std::cout << "Test case 5: " << getElementAtIndex(10) << std::endl;
    
    // Command line argument handling
    if (argc > 1) {
        int index = std::atoi(argv[1]);
        std::cout << "Element at index " << index << ": " << getElementAtIndex(index) << std::endl;
    }
    
    return 0;
}
