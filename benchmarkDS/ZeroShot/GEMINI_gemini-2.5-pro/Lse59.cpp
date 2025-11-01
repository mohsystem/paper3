#include <iostream>
#include <vector>    // Used for comparison, but not for the actual stack buffer
#include <algorithm> // For std::fill_n
#include <stdexcept> // For std::runtime_error

// A constant to prevent stack exhaustion from a very large allocation.
const int MAX_BUFFER_SIZE = 1024 * 10; // 10 KB is a safe limit for stack allocation

/**
 * Allocates a buffer on the stack and overwrites it with 'A'.
 * This function uses a Variable Length Array (VLA), which is a non-standard
 * extension in C++ but supported by compilers like GCC and Clang.
 * A check is performed to prevent allocating a buffer so large it would
 * likely cause a stack overflow crash.
 *
 * @param buffer_size The size of the buffer to allocate on the stack.
 */
void fillBuffer(int buffer_size) {
    if (buffer_size < 0) {
        std::cerr << "Error: Buffer size cannot be negative." << std::endl;
        return;
    }
    if (buffer_size > MAX_BUFFER_SIZE) {
        std::cerr << "Error: Requested buffer size " << buffer_size << " exceeds the maximum allowed size of " << MAX_BUFFER_SIZE << " for stack allocation." << std::endl;
        return;
    }
    
    // VLA: allocated on the stack. Its size is determined at runtime.
    // Note: VLAs are not part of the C++ standard.
    char buffer[buffer_size];

    // Securely fill the buffer without going out of bounds.
    // std::fill_n is a safe way to do this when the size is correct.
    if (buffer_size > 0) {
        std::fill_n(buffer, buffer_size, 'A');
    }
    
    std::cout << "Successfully allocated and filled a stack buffer of size " << buffer_size << "." << std::endl;
    // The buffer is automatically deallocated when the function returns.
}

int main() {
    std::cout << "--- C++ Test Cases ---" << std::endl;
    fillBuffer(16);          // Test Case 1: Small valid size
    fillBuffer(0);           // Test Case 2: Zero size
    fillBuffer(1024);        // Test Case 3: Larger valid size
    fillBuffer(-1);          // Test Case 4: Invalid negative size
    fillBuffer(MAX_BUFFER_SIZE + 1); // Test Case 5: Size exceeding the defined maximum
    return 0;
}