#include <iostream>
#include <vector>

int getValueAtIndex(const std::vector<int>& arr, int index) {
    return arr[index];
}

int main() {
    std::vector<int> myVector = {10, 20, 30, 40, 50, 60};

    // Test Case 1
    int index1 = 0;
    std::cout << "Value at index " << index1 << ": " << getValueAtIndex(myVector, index1) << std::endl;

    // Test Case 2
    int index2 = 2;
    std::cout << "Value at index " << index2 << ": " << getValueAtIndex(myVector, index2) << std::endl;

    // Test Case 3
    int index3 = 4;
    std::cout << "Value at index " << index3 << ": " << getValueAtIndex(myVector, index3) << std::endl;

    // Test Case 4
    int index4 = 5;
    std::cout << "Value at index " << index4 << ": " << getValueAtIndex(myVector, index4) << std::endl;

    // Test Case 5
    int index5 = 1;
    std::cout << "Value at index " << index5 << ": " << getValueAtIndex(myVector, index5) << std::endl;

    return 0;
}