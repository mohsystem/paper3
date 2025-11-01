
#include <iostream>
#include <cstdlib>

int getValueFromArray(int* array, int size, int index) {
    // Validate parameters to prevent security vulnerabilities
    if (array == nullptr) {
        return -1;
    }
    
    // Validate index is within valid bounds to prevent buffer overflow
    if (index < 0 || index >= size) {
        return -1;
    }
    
    return array[index];
}

int main() {
    // Define the id_sequence array
    int id_sequence[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    int size = sizeof(id_sequence) / sizeof(id_sequence[0]);
    
    // Test cases
    std::cout << "Test Case 1 - Index 0: " << getValueFromArray(id_sequence, size, 0) << std::endl;
    std::cout << "Test Case 2 - Index 5: " << getValueFromArray(id_sequence, size, 5) << std::endl;
    std::cout << "Test Case 3 - Index 9: " << getValueFromArray(id_sequence, size, 9) << std::endl;
    std::cout << "Test Case 4 - Index -1 (invalid): " << getValueFromArray(id_sequence, size, -1) << std::endl;
    std::cout << "Test Case 5 - Index 15 (out of bounds): " << getValueFromArray(id_sequence, size, 15) << std::endl;
    
    // Read input from user
    std::cout << "Enter an index: ";
    int index;
    
    if (std::cin >> index) {
        int result = getValueFromArray(id_sequence, size, index);
        return (result >= 0) ? result : 1;
    } else {
        return 1;
    }
}
