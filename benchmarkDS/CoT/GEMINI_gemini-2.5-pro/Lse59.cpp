#include <iostream>
#include <vector>    // Used for a safe alternative example in comments
#include <cstring>   // For memset

// Define a safe maximum for stack allocation to prevent stack overflow.
// Stack sizes vary, but 64KB is a reasonably safe and conservative limit.
const size_t MAX_STACK_ALLOC_SIZE = 65536;

/**
 * Allocates a buffer on the stack and fills it with 'A'.
 * Uses a Variable Length Array (VLA), which is a non-standard C++ extension
 * but commonly supported by compilers like GCC and Clang.
 *
 * @param buffer_size The size of the buffer to allocate.
 */
void allocateAndFillBuffer(size_t buffer_size) {
    // Security Check: Prevent stack overflow by validating the input size.
    if (buffer_size > MAX_STACK_ALLOC_SIZE) {
        std::cout << "Error: Requested buffer size " << buffer_size 
                  << " exceeds the maximum safe stack allocation size of " 
                  << MAX_STACK_ALLOC_SIZE << "." << std::endl;
        return;
    }

    if (buffer_size == 0) {
        std::cout << "Successfully handled request for buffer of size 0." << std::endl;
        return;
    }

    // Allocate the buffer on the stack using a VLA.
    char buffer[buffer_size];

    // Overwrite all values in the buffer with 'A'.
    // memset is a safe and efficient way to do this.
    std::memset(buffer, 'A', buffer_size);

    std::cout << "Successfully allocated and filled a stack buffer of size " << buffer_size << " with 'A'." << std::endl;
    // The buffer is automatically deallocated when the function returns.
}

int main() {
    std::cout << "--- C++ Test Cases ---" << std::endl;
    // Test Case 1: A small, typical size
    allocateAndFillBuffer(16);
    // Test Case 2: Size 1
    allocateAndFillBuffer(1);
    // Test Case 3: Size 0 (edge case)
    allocateAndFillBuffer(0);
    // Test Case 4: A larger, but valid size
    allocateAndFillBuffer(1024);
    // Test Case 5: A size that is too large and will be caught by our security check
    allocateAndFillBuffer(2000000); // This should fail safely
    return 0;
}