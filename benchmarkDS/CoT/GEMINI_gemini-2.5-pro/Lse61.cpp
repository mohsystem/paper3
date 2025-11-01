#include <iostream>

/**
 * WARNING: This function contains a deliberate double-free vulnerability.
 * It allocates a buffer and then frees it multiple times inside a loop.
 * This leads to undefined behavior and can crash the program or be exploited.
 * DO NOT use this pattern in production code.
 *
 * @param buffer_size The size of the buffer to allocate.
 * @param loop_count The number of times to free the buffer. Should be > 1 to cause a double-free.
 */
void vulnerable_double_free(size_t buffer_size, int loop_count) {
    if (buffer_size == 0 || loop_count <= 0) {
        std::cout << "Invalid size or loop count." << std::endl;
        return;
    }

    std::cout << "Allocating buffer of size: " << buffer_size << std::endl;
    char* buffer = new (std::nothrow) char[buffer_size];

    if (buffer == nullptr) {
        std::cerr << "Memory allocation failed." << std::endl;
        return;
    }

    std::cout << "Buffer allocated at: " << static_cast<void*>(buffer) << std::endl;

    for (int i = 0; i < loop_count; ++i) {
        std::cout << "Loop " << i + 1 << ": Freeing buffer." << std::endl;
        // The first call to delete[] is valid.
        // Any subsequent call is a double-free, which is undefined behavior.
        delete[] buffer;
    }
    std::cout << "Completed vulnerable function for buffer size " << buffer_size << "." << std::endl;
    std::cout << "------------------------------------" << std::endl;
}

int main() {
    // Test Cases
    // Note: The program will likely crash on the first test case with loop_count > 1.
    // The behavior is undefined and may vary between systems and compilers.
    vulnerable_double_free(1024, 2); 
    vulnerable_double_free(256, 3);
    vulnerable_double_free(1, 5);
    vulnerable_double_free(4096, 1); // This is safe, single free
    vulnerable_double_free(128, 10);

    return 0;
}