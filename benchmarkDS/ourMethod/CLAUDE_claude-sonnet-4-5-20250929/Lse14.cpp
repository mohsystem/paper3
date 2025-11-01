
#include <iostream>
#include <limits>
#include <string>

// Predefined array with fixed bounds
const int id_sequence[] = {100, 200, 300, 400, 500};
const size_t id_sequence_length = sizeof(id_sequence) / sizeof(id_sequence[0]);

/**
 * Retrieves value at specified index from id_sequence array.
 * Validates index to prevent out-of-bounds access.
 * 
 * @param index The index to retrieve from id_sequence
 * @param result Pointer to store the result value
 * @return true if successful, false if invalid index
 */
bool getValueAtIndex(int index, int* result) {
    // Input validation: Check null pointer
    if (result == nullptr) {
        std::cerr << "Error: Invalid result pointer" << std::endl;
        return false;
    }
    
    // Initialize output to safe value
    *result = 0;
    
    // Input validation: Check bounds to prevent array access violations
    // Reject negative indices and indices beyond array length
    // Cast to size_t only after validating non-negative
    if (index < 0 || static_cast<size_t>(index) >= id_sequence_length) {
        // Fail closed: return error without exposing internal details
        std::cerr << "Error: Invalid index" << std::endl;
        return false;
    }
    
    // Bounds checked: safe to access array
    *result = id_sequence[index];
    return true;
}

/**
 * Prompts user for index and returns corresponding value.
 * Validates all user input before processing.
 * 
 * @param result Pointer to store the result value
 * @return true if successful, false on error
 */
bool promptAndGetValue(int* result) {
    // Input validation: Check null pointer
    if (result == nullptr) {
        std::cerr << "Error: Invalid result pointer" << std::endl;
        return false;
    }
    
    // Initialize output to safe value
    *result = 0;
    
    try {
        std::cout << "Enter index (0-" << (id_sequence_length - 1) << "): ";
        
        int index;
        // Input validation: Check if input is valid integer
        if (!(std::cin >> index)) {
            // Clear error flags and discard invalid input
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\\n');
            std::cerr << "Error: Invalid input type" << std::endl;
            return false;
        }
        
        // Clear any remaining input
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\\n');
        
        // Delegate to validated function
        return getValueAtIndex(index, result);
        
    } catch (const std::exception&) {
        // Catch-all for unexpected errors, no internal details leaked
        std::cerr << "Error: Processing failed" << std::endl;
        return false;
    }
}

int main() {
    int value;
    
    // Test case 1: Valid index at start
    if (getValueAtIndex(0, &value)) {
        std::cout << "Test 1 - Index 0: " << value << std::endl;
    }
    
    // Test case 2: Valid index in middle
    if (getValueAtIndex(2, &value)) {
        std::cout << "Test 2 - Index 2: " << value << std::endl;
    }
    
    // Test case 3: Valid index at end
    if (getValueAtIndex(4, &value)) {
        std::cout << "Test 3 - Index 4: " << value << std::endl;
    }
    
    // Test case 4: Invalid negative index
    if (!getValueAtIndex(-1, &value)) {
        std::cout << "Test 4 - Index -1: Failed as expected" << std::endl;
    }
    
    // Test case 5: Invalid index beyond bounds
    if (!getValueAtIndex(10, &value)) {
        std::cout << "Test 5 - Index 10: Failed as expected" << std::endl;
    }
    
    return 0;
}
