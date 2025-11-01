#include <iostream>

/**
 * Returns the value from an array at a given index.
 *
 * @param arr The array of integers.
 * @param size The size of the array.
 * @param index The index of the element to retrieve.
 * @return The integer value at the specified index.
 */
int getValueFromArray(int arr[], int size, int index) {
    // Assuming the index is always within the valid bounds of the array.
    return arr[index];
}

int main() {
    int sampleArray[] = {10, 20, 30, 40, 50};
    int arraySize = sizeof(sampleArray) / sizeof(sampleArray[0]);

    // 5 Test Cases
    std::cout << "CPP Test Cases:" << std::endl;

    // Test Case 1
    std::cout << "Value at index 0: " << getValueFromArray(sampleArray, arraySize, 0) << std::endl;
    // Test Case 2
    std::cout << "Value at index 2: " << getValueFromArray(sampleArray, arraySize, 2) << std::endl;
    // Test Case 3
    std::cout << "Value at index 4: " << getValueFromArray(sampleArray, arraySize, 4) << std::endl;
    // Test Case 4
    std::cout << "Value at index 1: " << getValueFromArray(sampleArray, arraySize, 1) << std::endl;
    // Test Case 5
    std::cout << "Value at index 3: " << getValueFromArray(sampleArray, arraySize, 3) << std::endl;
    
    return 0;
}