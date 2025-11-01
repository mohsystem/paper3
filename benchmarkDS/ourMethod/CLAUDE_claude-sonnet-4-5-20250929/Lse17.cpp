
#include <iostream>
#include <string>
#include <limits>
#include <stdexcept>

// Predefined array for demonstration
const int VALUES[] = {10, 20, 30, 40, 50};
const size_t VALUES_SIZE = sizeof(VALUES) / sizeof(VALUES[0]);

/**
 * Retrieves the value at the specified index from the array.
 * Validates input to prevent out-of-bounds access.
 * 
 * @param index The index to retrieve
 * @param result Output parameter to store the result
 * @return true if successful, false if index is invalid
 */
bool getValueAtIndex(int index, int& result) {
    // Input validation: Check if index is within valid bounds
    // This prevents buffer overflow and out-of-bounds access
    if (index < 0 || static_cast<size_t>(index) >= VALUES_SIZE) {
        // Fail closed: return false for invalid index
        return false;
    }
    
    // Safe array access after bounds validation
    result = VALUES[index];
    return true;
}

int main() {
    try {
        // Test cases
        std::cout << "Running test cases:" << std::endl;
        int result;
        
        if (getValueAtIndex(0, result)) {
            std::cout << "Test 1 (index=0): " << result << std::endl;
        }
        if (getValueAtIndex(2, result)) {
            std::cout << "Test 2 (index=2): " << result << std::endl;
        }
        if (getValueAtIndex(4, result)) {
            std::cout << "Test 3 (index=4): " << result << std::endl;
        }
        if (!getValueAtIndex(-1, result)) {
            std::cout << "Test 4 (index=-1): Invalid" << std::endl;
        }
        if (!getValueAtIndex(10, result)) {
            std::cout << "Test 5 (index=10): Invalid" << std::endl;
        }
        
        // Interactive mode
        std::cout << "\\nEnter an index (0-" << (VALUES_SIZE - 1) << "): ";
        
        int userIndex;
        // Input validation: check if input is valid integer
        if (!(std::cin >> userIndex)) {
            // Generic error message - do not leak internal details
            std::cerr << "Error: Invalid input type. Expected integer." << std::endl;
            // Clear error flags and discard invalid input
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\\n');
            return 1;
        }
        
        if (getValueAtIndex(userIndex, result)) {
            std::cout << "Value at index " << userIndex << ": " << result << std::endl;
        } else {
            // Generic error message - do not leak internal details
            std::cerr << "Error: Index out of valid range." << std::endl;
        }
        
    } catch (const std::exception& e) {
        // Exception handling: catch and provide generic error message
        std::cerr << "Error: An unexpected error occurred." << std::endl;
        return 1;
    } catch (...) {
        // Catch-all for any other exceptions
        std::cerr << "Error: An unknown error occurred." << std::endl;
        return 1;
    }
    
    return 0;
}
