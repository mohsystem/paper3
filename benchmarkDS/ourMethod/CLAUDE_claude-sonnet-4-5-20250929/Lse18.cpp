
#include <iostream>
#include <array>

// Predefined array for demonstration
const std::array<int, 10> DATA_ARRAY = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

/**
 * Returns the value at the specified index in the array.
 * Validates input to prevent out-of-bounds access.
 * 
 * @param index The index to retrieve from the array
 * @return The value at the specified index, or -1 if invalid
 */
int getValueAtIndex(int index) {
    // Input validation: Check if index is within valid bounds
    // This prevents undefined behavior from out-of-bounds access
    if (index < 0 || static_cast<size_t>(index) >= DATA_ARRAY.size()) {
        // Fail closed: return error indicator without exposing internal details
        return -1;
    }
    
    // Safe array access after validation using std::array which provides bounds checking
    return DATA_ARRAY[static_cast<size_t>(index)];
}

int main() {
    // Test case 1: Valid index at beginning
    int result1 = getValueAtIndex(0);
    std::cout << "Test 1 - Index 0: " << result1 << std::endl;
    
    // Test case 2: Valid index in middle
    int result2 = getValueAtIndex(5);
    std::cout << "Test 2 - Index 5: " << result2 << std::endl;
    
    // Test case 3: Valid index at end
    int result3 = getValueAtIndex(9);
    std::cout << "Test 3 - Index 9: " << result3 << std::endl;
    
    // Test case 4: Invalid negative index
    int result4 = getValueAtIndex(-1);
    std::cout << "Test 4 - Index -1: " << result4 << std::endl;
    
    // Test case 5: Invalid index beyond array bounds
    int result5 = getValueAtIndex(15);
    std::cout << "Test 5 - Index 15: " << result5 << std::endl;
    
    return 0;
}
