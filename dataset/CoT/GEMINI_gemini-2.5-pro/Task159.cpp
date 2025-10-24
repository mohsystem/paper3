#include <iostream>
#include <vector>    // Using vector for safer, automatic memory management
#include <optional>  // Using std::optional for safe return values

/**
 * Reads data from a buffer at a given index.
 *
 * @param buffer The dynamically allocated buffer.
 * @param size The size of the buffer.
 * @param index The index from which to read data.
 * @return An std::optional containing the data if the index is valid, otherwise std::nullopt.
 */
std::optional<int> readFromBuffer(const int* buffer, size_t size, int index) {
    // Security: Check if the pointer is null or if the index is out of bounds.
    // This prevents segmentation faults and reading from invalid memory.
    if (buffer == nullptr || index < 0 || static_cast<size_t>(index) >= size) {
        // Index is out of bounds or buffer is invalid.
        return std::nullopt;
    } else {
        return buffer[index];
    }
}

int main() {
    const size_t BUFFER_SIZE = 10;
    // 1. Dynamically allocate a memory buffer.
    int* buffer = new (std::nothrow) int[BUFFER_SIZE];
    
    // Security: Check if memory allocation was successful.
    if (buffer == nullptr) {
        std::cerr << "Error: Memory allocation failed." << std::endl;
        return 1;
    }

    // 2. Fill the buffer with some data.
    for (size_t i = 0; i < BUFFER_SIZE; ++i) {
        buffer[i] = i * 10; // e.g., 0, 10, 20, ...
    }

    std::cout << "C++ Test Cases:" << std::endl;
    
    // 3. Test cases to read from the buffer.
    int testIndices[] = {3, 0, 9, 10, -1};

    for (int i = 0; i < 5; ++i) {
        int index = testIndices[i];
        std::cout << "Test " << (i + 1) << ": Reading at index " << index << "... ";
        
        std::optional<int> result = readFromBuffer(buffer, BUFFER_SIZE, index);

        if (result.has_value()) {
            std::cout << "Success! Value: " << result.value() << std::endl;
        } else {
            std::cout << "Error: Index is out of bounds." << std::endl;
        }
    }

    // Security: Deallocate the memory to prevent memory leaks.
    delete[] buffer;
    buffer = nullptr; // Good practice to nullify pointer after deletion.

    return 0;
}