
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cstdlib>

void printElementAtIndex(const std::vector<std::string>& array, const std::string& indexStr) {
    if (indexStr.empty()) {
        std::cout << "Error: Index argument is empty or null" << std::endl;
        return;
    }
    
    try {
        size_t pos;
        int index = std::stoi(indexStr, &pos);
        
        if (pos != indexStr.length()) {
            std::cout << "Error: Invalid integer format: " << indexStr << std::endl;
            return;
        }
        
        if (index < 0 || index >= static_cast<int>(array.size())) {
            std::cout << "Error: Index " << index << " is out of bounds. Valid range: 0-" 
                      << (array.size() - 1) << std::endl;
            return;
        }
        
        std::cout << "Element at index " << index << ": " << array[index] << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cout << "Error: Invalid integer format: " << indexStr << std::endl;
    } catch (const std::out_of_range& e) {
        std::cout << "Error: Number too large: " << indexStr << std::endl;
    }
}

int main() {
    std::vector<std::string> elements = {"apple", "banana", "cherry", "date", "elderberry", "fig", "grape"};
    
    // Test cases
    std::cout << "Test Case 1: Valid index 0" << std::endl;
    printElementAtIndex(elements, "0");
    
    std::cout << "\\nTest Case 2: Valid index 3" << std::endl;
    printElementAtIndex(elements, "3");
    
    std::cout << "\\nTest Case 3: Negative index" << std::endl;
    printElementAtIndex(elements, "-1");
    
    std::cout << "\\nTest Case 4: Out of bounds index" << std::endl;
    printElementAtIndex(elements, "10");
    
    std::cout << "\\nTest Case 5: Invalid format" << std::endl;
    printElementAtIndex(elements, "abc");
    
    return 0;
}
