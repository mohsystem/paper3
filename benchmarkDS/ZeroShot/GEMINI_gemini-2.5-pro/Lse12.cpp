#include <iostream>

/**
 * Gets a value from an array at a specified index.
 * Securely checks if the index is within the valid bounds of the array.
 *
 * @param arr Pointer to the source array of integers.
 * @param size The size of the array.
 * @param index The index from which to retrieve the value.
 * @return The value at the given index, or -1 if the index is out of bounds.
 */
int getValueFromArray(const int* arr, int size, int index) {
    // Secure: Validate that the pointer is not null and the index is within bounds.
    // This prevents segmentation faults from null pointers or out-of-bounds access.
    if (arr == nullptr || index < 0 || index >= size) {
        std::cerr << "Error: Index " << index << " is out of bounds for array of size " << size << "." << std::endl;
        return -1; // Return a designated error code.
    }
    return arr[index];
}

int main() {
    int data[] = {10, 20, 30, 40, 50};
    int size = sizeof(data) / sizeof(data[0]);

    std::cout << "Running 5 test cases..." << std::endl;

    // Test Case 1: Valid index in the middle
    int index1 = 2;
    std::cout << "\nTest Case 1: index = " << index1 << std::endl;
    int value1 = getValueFromArray(data, size, index1);
    std::cout << "Value returned: " << value1 << std::endl; // Expected: 30

    // Test Case 2: Valid index at the beginning
    int index2 = 0;
    std::cout << "\nTest Case 2: index = " << index2 << std::endl;
    int value2 = getValueFromArray(data, size, index2);
    std::cout << "Value returned: " << value2 << std::endl; // Expected: 10

    // Test Case 3: Invalid negative index
    int index3 = -1;
    std::cout << "\nTest Case 3: index = " << index3 << std::endl;
    int value3 = getValueFromArray(data, size, index3);
    std::cout << "Value returned: " << value3 << std::endl; // Expected: -1

    // Test Case 4: Invalid index equal to size
    int index4 = size;
    std::cout << "\nTest Case 4: index = " << index4 << std::endl;
    int value4 = getValueFromArray(data, size, index4);
    std::cout << "Value returned: " << value4 << std::endl; // Expected: -1

    // Test Case 5: Valid index at the end
    int index5 = 4;
    std::cout << "\nTest Case 5: index = " << index5 << std::endl;
    int value5 = getValueFromArray(data, size, index5);
    std::cout << "Value returned: " << value5 << std::endl; // Expected: 50
    
    return 0;
}