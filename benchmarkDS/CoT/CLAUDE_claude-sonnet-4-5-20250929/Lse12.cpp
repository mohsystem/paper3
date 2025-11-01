
#include <iostream>
#include <cstdlib>

int getValueFromArray(int* array, int size, int index) {
    // Security check: validate index to prevent out-of-bounds access
    if (array == nullptr || index < 0 || index >= size) {
        return -1; // Return error code for invalid access
    }
    return array[index];
}

int main() {
    // Test cases
    int testArray1[] = {10, 20, 30, 40, 50};
    int size = 5;
    
    // Test case 1: Valid index in middle
    std::cout << "Test 1: " << getValueFromArray(testArray1, size, 2) << std::endl; // Expected: 30
    
    // Test case 2: Valid index at start
    std::cout << "Test 2: " << getValueFromArray(testArray1, size, 0) << std::endl; // Expected: 10
    
    // Test case 3: Valid index at end
    std::cout << "Test 3: " << getValueFromArray(testArray1, size, 4) << std::endl; // Expected: 50
    
    // Test case 4: Invalid index (negative)
    std::cout << "Test 4: " << getValueFromArray(testArray1, size, -1) << std::endl; // Expected: -1
    
    // Test case 5: Invalid index (out of bounds)
    std::cout << "Test 5: " << getValueFromArray(testArray1, size, 10) << std::endl; // Expected: -1
    
    // Original functionality: read from standard input
    int index;
    if (std::cin >> index) {
        int result = getValueFromArray(testArray1, size, index);
        return (result >= 0) ? result : 1;
    }
    
    return 1;
}
