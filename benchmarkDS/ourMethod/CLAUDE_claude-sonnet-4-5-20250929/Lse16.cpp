
#include <iostream>
#include <array>
#include <limits>

// Predefined array with values
const std::array<int, 10> VALUES = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

/**
 * Retrieves the value at the specified index from the array.
 * Validates input to prevent out-of-bounds access.
 * 
 * @param index The index to retrieve
 * @return The value at the specified index, or -1 if invalid
 */
int getValueAtIndex(int index) {
    // Input validation: check if index is within valid range
    // This prevents buffer overflow/underflow (CWE-129, CWE-125)
    if (index < 0 || static_cast<size_t>(index) >= VALUES.size()) {
        // Fail closed: return error indicator without exposing internal details
        std::cerr << "Error: Invalid index" << std::endl;
        return -1;
    }
    
    // Safe array access after validation using bounds-checked container
    return VALUES[static_cast<size_t>(index)];
}

int main() {
    // Test cases with boundary and normal conditions
    std::cout << "Test Case 1 (index=0): " << getValueAtIndex(0) << std::endl;      // Expected: 10
    std::cout << "Test Case 2 (index=5): " << getValueAtIndex(5) << std::endl;      // Expected: 60
    std::cout << "Test Case 3 (index=9): " << getValueAtIndex(9) << std::endl;      // Expected: 100
    std::cout << "Test Case 4 (index=-1): " << getValueAtIndex(-1) << std::endl;    // Expected: -1 (error)
    std::cout << "Test Case 5 (index=10): " << getValueAtIndex(10) << std::endl;    // Expected: -1 (error)
    
    return 0;
}
