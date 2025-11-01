
#include <iostream>
#include <cstdlib>

int getValueFromArray(int array[], int size, int index) {
    if (index >= 0 && index < size) {
        return array[index];
    }
    return -1;
}

int main() {
    int id_sequence[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    int size = sizeof(id_sequence) / sizeof(id_sequence[0]);
    
    // Test case 1
    int index1 = 0;
    int result1 = getValueFromArray(id_sequence, size, index1);
    std::cout << "Test 1 - Index " << index1 << ": " << result1 << std::endl;
    
    // Test case 2
    int index2 = 5;
    int result2 = getValueFromArray(id_sequence, size, index2);
    std::cout << "Test 2 - Index " << index2 << ": " << result2 << std::endl;
    
    // Test case 3
    int index3 = 9;
    int result3 = getValueFromArray(id_sequence, size, index3);
    std::cout << "Test 3 - Index " << index3 << ": " << result3 << std::endl;
    
    // Test case 4
    int index4 = 3;
    int result4 = getValueFromArray(id_sequence, size, index4);
    std::cout << "Test 4 - Index " << index4 << ": " << result4 << std::endl;
    
    // Test case 5
    int index5 = 7;
    int result5 = getValueFromArray(id_sequence, size, index5);
    std::cout << "Test 5 - Index " << index5 << ": " << result5 << std::endl;
    
    return result5;
}
