#include <iostream>
#include <vector>
#include <stdexcept>

/**
 * Securely reads a value from the buffer (std::vector).
 * The .at() method of std::vector provides automatic bounds checking.
 *
 * @param buffer The vector to read from.
 * @param index The index to read.
 * @param outValue A reference to store the output value.
 * @return true if the read was successful, false otherwise.
 */
bool secureRead(const std::vector<char>& buffer, int index, char& outValue) {
    // A negative index is always out of bounds for a vector.
    if (index < 0) {
        return false;
    }
    
    // Convert to size_t for safe comparison and access.
    size_t s_index = static_cast<size_t>(index);
    
    // Secure bounds check
    if (s_index < buffer.size()) {
        outValue = buffer[s_index]; // or buffer.at(s_index)
        return true;
    }
    
    return false;
}

int main() {
    const int bufferSize = 10;
    // 1. Dynamically allocate a memory buffer using std::vector
    std::vector<char> buffer(bufferSize);

    // Initialize buffer with some data
    for (int i = 0; i < bufferSize; ++i) {
        buffer[i] = static_cast<char>(i * 10);
    }

    std::cout << "Buffer created with size: " << bufferSize << std::endl;
    std::cout << "Buffer content: [0, 10, 20, 30, 40, 50, 60, 70, 80, 90]" << std::endl;
    std::cout << "--- Running CPP Test Cases ---" << std::endl;

    // 2. Test cases
    int testIndices[] = {0, 5, 9, -1, 10};

    for (int index : testIndices) {
        char value;
        if (secureRead(buffer, index, value)) {
            // Cast to int for printing the numeric value
            std::cout << "Reading at index " << index << ": Success! Value = " << static_cast<int>(value) << std::endl;
        } else {
            std::cout << "Reading at index " << index << ": Error! Index is out of bounds." << std::endl;
        }
    }

    // No need to manually free memory, std::vector handles it automatically (RAII).
    return 0;
}