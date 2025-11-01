
#include <iostream>
#include <cstdlib>

int getValueFromArray(int* array, int size, int index) {
    // Security: Validate index to prevent out-of-bounds access
    if (array == nullptr || index < 0 || index >= size) {
        return -1; // Return error code for invalid access
    }
    return array[index];
}

int main() {
    int id_sequence[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    int size = sizeof(id_sequence) / sizeof(id_sequence[0]);
    
    // Test cases
    std::cout << "Test Case 1 (index 0): " << getValueFromArray(id_sequence, size, 0) << std::endl;
    std::cout << "Test Case 2 (index 5): " << getValueFromArray(id_sequence, size, 5) << std::endl;
    std::cout << "Test Case 3 (index 9): " << getValueFromArray(id_sequence, size, 9) << std::endl;
    std::cout << "Test Case 4 (index -1): " << getValueFromArray(id_sequence, size, -1) << std::endl;
    std::cout << "Test Case 5 (index 15): " << getValueFromArray(id_sequence, size, 15) << std::endl;
    
    // Read from standard input
    std::cout << "Enter an index: ";
    int index;
    if (std::cin >> index) {
        int result = getValueFromArray(id_sequence, size, index);
        return (result >= 0) ? result : 1;
    }
    return 1;
}
