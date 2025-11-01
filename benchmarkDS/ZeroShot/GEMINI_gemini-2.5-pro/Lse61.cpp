#include <iostream>
#include <new> // For std::nothrow

class Lse61 {
public:
    /**
     * WARNING: This function demonstrates a dangerous double-free vulnerability (CWE-415).
     * Calling delete[] on the same memory pointer more than once leads to
     * undefined behavior, which can include memory corruption, security
     * vulnerabilities, or a program crash.
     * This code is for educational purposes only. DO NOT use in production.
     * A secure practice is to set the pointer to nullptr after freeing the memory.
     */
    static void demonstrateDoubleFree(size_t buffer_size) {
        std::cout << "--- Demonstrating Double Free with buffer_size = " << buffer_size << " ---" << std::endl;
        
        // 1. Allocate a buffer
        // Using char as it represents a single byte.
        // std::nothrow prevents an exception on allocation failure.
        char* buffer = new (std::nothrow) char[buffer_size];
        
        if (buffer == nullptr) {
            std::cout << "Memory allocation failed." << std::endl;
            std::cout << "--------------------------------------------------" << std::endl << std::endl;
            return;
        }

        std::cout << "Buffer allocated at address: " << static_cast<void*>(buffer) << std::endl;

        // 2. Free the buffer in a loop (demonstrating the vulnerability)
        // The program will likely crash or exhibit undefined behavior on the second iteration.
        for (int i = 0; i < 2; ++i) {
            std::cout << "Attempting to free buffer (iteration " << i + 1 << ")..." << std::endl;
            delete[] buffer;
            std::cout << "delete[] call completed." << std::endl;
        }
        
        std::cout << "--------------------------------------------------" << std::endl << std::endl;
    }
};

int main() {
    // NOTE: The program will likely crash during the first test case
    // that successfully allocates memory. Subsequent test cases may not run.
    std::cout << "--- Starting C++ Double-Free Demonstration ---" << std::endl;
    std::cout << "WARNING: This program is expected to crash due to a double-free vulnerability." << std::endl << std::endl;
    
    Lse61::demonstrateDoubleFree(16);
    Lse61::demonstrateDoubleFree(32);
    Lse61::demonstrateDoubleFree(64);
    Lse61::demonstrateDoubleFree(0); // new char[0] is valid, returns a unique pointer.
    Lse61::demonstrateDoubleFree(128);

    std::cout << "--- Demonstration Finished (This message will not be reached) ---" << std::endl;
    
    return 0;
}