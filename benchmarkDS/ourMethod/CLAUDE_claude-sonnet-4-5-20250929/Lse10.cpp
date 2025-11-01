
#include <iostream>
#include <cstdlib>
#include <limits>

// Function to get value from array with bounds checking
// Returns the value at the given index if valid, otherwise returns -1
int getValueFromArray(const int* array, int size, int index) {
    // Validate input parameters to prevent out-of-bounds access
    if (array == nullptr) {
        std::cerr << "Error: Array is null" << std::endl;
        return -1;
    }
    
    // Ensure size is non-negative to prevent unsigned comparison issues
    if (size < 0) {
        std::cerr << "Error: Invalid size" << std::endl;
        return -1;
    }
    
    // Ensure index is within valid bounds [0, size-1]
    if (index < 0 || index >= size) {
        std::cerr << "Error: Index out of bounds" << std::endl;
        return -1;
    }
    
    // Safe to access array at this point
    return array[index];
}

int main() {
    // Predefined array for testing
    const int id_sequence[] = {10, 20, 30, 40, 50};
    const int size = sizeof(id_sequence) / sizeof(id_sequence[0]);
    
    int index;
    
    // Read index from standard input with validation
    if (!(std::cin >> index)) {
        std::cerr << "Error: Invalid input" << std::endl;
        return 1;
    }
    
    // Clear any remaining input to prevent issues
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\\n');
    
    // Get value from array with bounds checking
    int result = getValueFromArray(id_sequence, size, index);
    
    // Exit with the retrieved value (or error code -1)
    // Ensure exit code is in valid range [0, 255]
    int exit_code = (result >= 0 && result <= 255) ? result : (result & 0xFF);
    return exit_code;
    
    // Test cases (uncomment to run)
    /*
    std::cout << "Test 1 (index 0): " << getValueFromArray(id_sequence, size, 0) << std::endl;  // Expected: 10
    std::cout << "Test 2 (index 2): " << getValueFromArray(id_sequence, size, 2) << std::endl;  // Expected: 30
    std::cout << "Test 3 (index 4): " << getValueFromArray(id_sequence, size, 4) << std::endl;  // Expected: 50
    std::cout << "Test 4 (index -1): " << getValueFromArray(id_sequence, size, -1) << std::endl;  // Expected: -1
    std::cout << "Test 5 (index 10): " << getValueFromArray(id_sequence, size, 10) << std::endl;  // Expected: -1
    return 0;
    */
}
