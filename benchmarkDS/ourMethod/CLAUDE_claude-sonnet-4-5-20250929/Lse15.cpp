
#include <iostream>
#include <stdexcept>
#include <limits>
#include <string>

// Predefined array of id_sequence values
const int ID_SEQUENCE[] = {100, 200, 300, 400, 500, 600, 700, 800, 900, 1000};
const size_t ID_SEQUENCE_LENGTH = sizeof(ID_SEQUENCE) / sizeof(ID_SEQUENCE[0]);

/**
 * Retrieves value at specified index from id_sequence array
 * @param index The index to retrieve (must be within array bounds)
 * @return The value at the specified index
 * @throws std::out_of_range if index is out of bounds
 */
int getValueAtIndex(int index) {
    // Input validation: check if index is negative
    if (index < 0) {
        throw std::out_of_range("Invalid index: must be non-negative");
    }
    
    // Input validation: check if index is within bounds using size_t comparison
    // to avoid signed/unsigned comparison issues
    if (static_cast<size_t>(index) >= ID_SEQUENCE_LENGTH) {
        throw std::out_of_range("Invalid index: must be between 0 and " + 
                                std::to_string(ID_SEQUENCE_LENGTH - 1));
    }
    
    // Return value at valid index
    return ID_SEQUENCE[index];
}

/**
 * Prompts user for index and returns corresponding value
 * @return The value at user-specified index, or -1 on error
 */
int promptAndGetValue() {
    try {
        std::cout << "Enter an index value (0-" << (ID_SEQUENCE_LENGTH - 1) << "): ";
        
        int index;
        // Validate input is an integer
        if (!(std::cin >> index)) {
            std::cerr << "Error: Invalid input type" << std::endl;
            // Clear error flags and ignore bad input
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\\n');
            return -1;
        }
        
        return getValueAtIndex(index);
        
    } catch (const std::out_of_range& e) {
        // Handle out of bounds error without exposing internals
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    } catch (const std::exception& e) {
        // Catch any other exceptions
        std::cerr << "Error: Invalid input" << std::endl;
        return -1;
    }
}

int main() {
    // Test case 1: Valid index at start
    std::cout << "\\nTest case 1: Valid index 0" << std::endl;
    try {
        int result = getValueAtIndex(0);
        std::cout << "Value at index 0: " << result << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 1 failed: " << e.what() << std::endl;
    }
    
    // Test case 2: Valid index in middle
    std::cout << "\\nTest case 2: Valid index 5" << std::endl;
    try {
        int result = getValueAtIndex(5);
        std::cout << "Value at index 5: " << result << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 2 failed: " << e.what() << std::endl;
    }
    
    // Test case 3: Valid index at end
    std::cout << "\\nTest case 3: Valid index 9" << std::endl;
    try {
        int result = getValueAtIndex(9);
        std::cout << "Value at index 9: " << result << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 3 failed: " << e.what() << std::endl;
    }
    
    // Test case 4: Invalid negative index
    std::cout << "\\nTest case 4: Invalid index -1" << std::endl;
    try {
        int result = getValueAtIndex(-1);
        std::cout << "Value at index -1: " << result << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 4 failed (expected): " << e.what() << std::endl;
    }
    
    // Test case 5: Invalid out of bounds index
    std::cout << "\\nTest case 5: Invalid index 15" << std::endl;
    try {
        int result = getValueAtIndex(15);
        std::cout << "Value at index 15: " << result << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 5 failed (expected): " << e.what() << std::endl;
    }
    
    return 0;
}
