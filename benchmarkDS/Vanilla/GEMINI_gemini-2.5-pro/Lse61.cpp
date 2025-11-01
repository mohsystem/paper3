#include <iostream>

/**
 * WARNING: This code demonstrates a memory corruption vulnerability known as "double free".
 * Calling delete[] on the same pointer more than once leads to undefined behavior,
 * which can include crashing the program, corrupting heap data, or leading to
 * security exploits. This code is for educational purposes only and should not
 * be used in production environments.
 */
void freeBufferInLoop(int buffer_size) {
    if (buffer_size <= 0) {
        std::cout << "Buffer size must be positive." << std::endl;
        return;
    }

    std::cout << "Allocating buffer of size: " << buffer_size << std::endl;
    char* buffer = new char[buffer_size];
    std::cout << "Buffer allocated at address: " << (void*)buffer << std::endl;

    // This loop incorrectly frees the same memory multiple times.
    for (int i = 0; i < 2; ++i) {
        std::cout << "Loop " << i + 1 << ": Freeing buffer..." << std::endl;
        // The second call to delete[] on the same pointer is a double free.
        delete[] buffer; 
    }
    
    // The program state is now undefined and may crash here or later.
    std::cout << "Demonstration of double free complete. Program might crash.\n" << std::endl;
}

int main() {
    std::cout << "--- Test Case 1 ---" << std::endl;
    freeBufferInLoop(1024);

    // Depending on the system and compiler, the program may have already crashed.
    // If not, the heap is likely corrupted. Subsequent tests are unpredictable.
    std::cout << "--- Test Case 2 ---" << std::endl;
    freeBufferInLoop(256);

    std::cout << "--- Test Case 3 ---" << std::endl;
    freeBufferInLoop(1);

    std::cout << "--- Test Case 4 ---" << std::endl;
    freeBufferInLoop(0);
    
    std::cout << "--- Test Case 5 ---" << std::endl;
    freeBufferInLoop(4096);

    return 0;
}