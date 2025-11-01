
#include <iostream>
#include <cstdlib>
#include <limits>

// Returns the value at the given index from the array
// Security: Validates all parameters to prevent buffer overflow/underflow
int getValueFromArray(const int* array, int size, int index) {
    // Validate array pointer is not NULL
    if (array == nullptr) {
        std::cerr << "Error: Null array provided" << std::endl;
        return -1;
    }
    
    // Validate size is positive to prevent negative indexing
    if (size <= 0) {
        std::cerr << "Error: Invalid size" << std::endl;
        return -1;
    }
    
    // Bounds checking: ensure index is within valid range [0, size)
    // This prevents both buffer underflow (negative) and overflow (>= size)
    if (index < 0 || index >= size) {
        std::cerr << "Error: Index out of bounds" << std::endl;
        return -1;
    }
    
    return array[index];
}

int main() {
    // Predefined test array - fixed size for security
    const int testArray[] = {10, 20, 30, 40, 50};
    const int size = sizeof(testArray) / sizeof(testArray[0]);
    
    int index = 0;
    
    // Read index from standard input - treat as untrusted
    if (!(std::cin >> index)) {
        // Input validation failed
        std::cerr << "Error: Invalid input" << std::endl;
        return 1;
    }
    
    // Clear any remaining input to prevent issues
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\\n');
    
    // Call function with validated parameters
    int result = getValueFromArray(testArray, size, index);
    
    // Return result as exit code (constrained to valid range 0-255)
    // Use absolute value and modulo to ensure valid exit code
    return std::abs(result) % 256;
}

/*
Test cases (compile and run with different inputs):
Input: 2  -> Expected output: exit code 30
Input: 0  -> Expected output: exit code 10
Input: 4  -> Expected output: exit code 50
Input: -1 -> Expected output: exit code 1 (error)
Input: 10 -> Expected output: exit code 1 (error)
*/
