
#include <iostream>
#include <string>
#include <vector>

std::string getElementAtIndex(int index) {
    std::vector<std::string> elements = {"apple", "banana", "cherry", "date", "elderberry"};
    
    if (index < 0 || index >= static_cast<int>(elements.size())) {
        return "Error: Index out of bounds";
    }
    
    return elements[index];
}

int main(int argc, char* argv[]) {
    // Test cases
    std::cout << "Test Case 1: " << getElementAtIndex(0) << std::endl;
    std::cout << "Test Case 2: " << getElementAtIndex(2) << std::endl;
    std::cout << "Test Case 3: " << getElementAtIndex(4) << std::endl;
    std::cout << "Test Case 4: " << getElementAtIndex(-1) << std::endl;
    std::cout << "Test Case 5: " << getElementAtIndex(10) << std::endl;
    
    // Command line argument processing
    if (argc > 1) {
        try {
            int index = std::stoi(argv[1]);
            std::cout << "\\nCommand line result: " << getElementAtIndex(index) << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Error: Invalid number format" << std::endl;
        }
    }
    
    return 0;
}
