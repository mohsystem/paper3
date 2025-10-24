#include <iostream>
#include <vector>

/**
 * Reads an integer value from a dynamically allocated buffer at a given index.
 *
 * @param buffer The pointer to the integer buffer.
 * @param size   The size of the buffer.
 * @param index  The index from which to read the data.
 * @return The value at the specified index, or -1 if the index is out of bounds.
 */
int readFromBuffer(const int* buffer, int size, int index) {
    if (buffer == nullptr || index < 0 || index >= size) {
        std::cerr << "Error: Index " << index << " is out of bounds." << std::endl;
        return -1; // Return a sentinel value for an invalid index
    }
    return buffer[index];
}

void printBuffer(const int* buffer, int size) {
    std::cout << "Buffer created: [";
    for (int i = 0; i < size; ++i) {
        std::cout << buffer[i] << (i == size - 1 ? "" : ", ");
    }
    std::cout << "]" << std::endl;
}


int main() {
    int bufferSize = 10;

    // 1. Dynamically allocate a memory buffer
    int* dynamicBuffer = new int[bufferSize];

    // 2. Initialize the buffer with some data
    for (int i = 0; i < bufferSize; ++i) {
        dynamicBuffer[i] = i * 10;
    }
    printBuffer(dynamicBuffer, bufferSize);
    std::cout << "------------------------------------" << std::endl;

    // 3. Test cases to read data from the buffer
    // Test Case 1: Read from a valid index at the beginning
    int index1 = 0;
    int value1 = readFromBuffer(dynamicBuffer, bufferSize, index1);
    std::cout << "Reading from index " << index1 << ": " << value1 << std::endl;

    // Test Case 2: Read from a valid index in the middle
    int index2 = 5;
    int value2 = readFromBuffer(dynamicBuffer, bufferSize, index2);
    std::cout << "Reading from index " << index2 << ": " << value2 << std::endl;

    // Test Case 3: Read from a valid index at the end
    int index3 = bufferSize - 1;
    int value3 = readFromBuffer(dynamicBuffer, bufferSize, index3);
    std::cout << "Reading from index " << index3 << ": " << value3 << std::endl;

    // Test Case 4: Read from an invalid index (negative)
    int index4 = -1;
    int value4 = readFromBuffer(dynamicBuffer, bufferSize, index4);
    std::cout << "Reading from index " << index4 << ": " << value4 << std::endl;

    // Test Case 5: Read from an invalid index (out of bounds)
    int index5 = bufferSize;
    int value5 = readFromBuffer(dynamicBuffer, bufferSize, index5);
    std::cout << "Reading from index " << index5 << ": " << value5 << std::endl;

    // 4. Deallocate the memory to prevent memory leaks
    delete[] dynamicBuffer;
    dynamicBuffer = nullptr; // Good practice to nullify pointer after deletion

    return 0;
}