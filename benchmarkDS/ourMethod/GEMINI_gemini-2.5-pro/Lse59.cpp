#include <iostream>
#include <algorithm>
#include <cstddef>

// Define a safe maximum buffer size to prevent stack overflow.
const size_t MAX_BUFFER_SIZE = 4096;

/**
 * @brief Allocates a buffer on the stack and overwrites it with 'A'.
 * 
 * Uses a Variable Length Array (VLA), which is a non-standard C++ extension.
 * The buffer size is validated to prevent stack exhaustion.
 * 
 * @param buffer_size The desired size of the buffer. Must be > 0 and <= MAX_BUFFER_SIZE.
 */
void overwrite_buffer(size_t buffer_size) {
    if (buffer_size == 0 || buffer_size > MAX_BUFFER_SIZE) {
        std::cerr << "Error: Invalid buffer size. Must be between 1 and "
                  << MAX_BUFFER_SIZE << "." << std::endl;
        return;
    }

    // VLA: This is a compiler extension in C++, not standard.
    char buffer[buffer_size];

    std::fill_n(buffer, buffer_size, 'A');

    std::cout << "Successfully allocated and overwrote a buffer of size " << buffer_size << "." << std::endl;
    // Basic check to confirm the operation
    if (buffer_size > 0 && (buffer[0] != 'A' || buffer[buffer_size - 1] != 'A')) {
        std::cerr << "Buffer content verification failed." << std::endl;
    }
}

int main() {
    std::cout << "--- Test Case 1: Valid size (16) ---" << std::endl;
    overwrite_buffer(16);

    std::cout << "\n--- Test Case 2: Edge case size (1) ---" << std::endl;
    overwrite_buffer(1);
    
    std::cout << "\n--- Test Case 3: Maximum valid size (" << MAX_BUFFER_SIZE << ") ---" << std::endl;
    overwrite_buffer(MAX_BUFFER_SIZE);

    std::cout << "\n--- Test Case 4: Invalid size (0) ---" << std::endl;
    overwrite_buffer(0);

    std::cout << "\n--- Test Case 5: Invalid size (too large) ---" << std::endl;
    overwrite_buffer(MAX_BUFFER_SIZE + 1);

    return 0;
}