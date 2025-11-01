
#include <iostream>
#include <limits>
#include <cstdlib>

// Function to get value from array with bounds checking
// Returns the value at the given index if valid, otherwise returns -1
int getValueFromArray(const int* array, int size, int index) {
    // Security: Validate array is not NULL
    if (array == nullptr) {
        std::cerr << "Error: Array is NULL" << std::endl;
        return -1;
    }
    
    // Security: Validate size is non-negative
    if (size < 0) {
        std::cerr << "Error: Invalid size" << std::endl;
        return -1;
    }
    
    // Security: Bounds checking to prevent out-of-bounds access
    if (index < 0 || index >= size) {
        std::cerr << "Error: Index out of bounds" << std::endl;
        return -1;
    }
    
    // Safe to access array element after validation
    return array[index];
}

int main() {
    // Predefined array for testing
    const int id_sequence[] = {10, 20, 30, 40, 50};
    const int size = sizeof(id_sequence) / sizeof(id_sequence[0]);
    
    std::cout << "Enter index: ";
    
    int index;
    
    // Security: Validate input is an integer
    if (!(std::cin >> index)) {
        std::cerr << "Error: Invalid input" << std::endl;
        return 1;
    }
    
    // Security: Clear any remaining input to prevent injection
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\\n');
    
    // Call function and get result
    int result = getValueFromArray(id_sequence, size, index);
    
    // Return the result as exit code (values outside 0-255 may be truncated)
    return (result >= 0 && result <= 255) ? result : 1;
}

// Test cases (create separate test function)
/*
void runTests() {
    const int id_sequence[] = {10, 20, 30, 40, 50};
    const int size = sizeof(id_sequence) / sizeof(id_sequence[0]);
    
    std::cout << "Test 1 - Valid index 0: " << getValueFromArray(id_sequence, size, 0) << std::endl; // Expected: 10
    std::cout << "Test 2 - Valid index 4: " << getValueFromArray(id_sequence, size, 4) << std::endl; // Expected: 50
    std::cout << "Test 3 - Invalid index -1: " << getValueFromArray(id_sequence, size, -1) << std::endl; // Expected: -1
    std::cout << "Test 4 - Invalid index 5: " << getValueFromArray(id_sequence, size, 5) << std::endl; // Expected: -1
    std::cout << "Test 5 - NULL array: " << getValueFromArray(nullptr, 0, 0) << std::endl; // Expected: -1
}
*/
