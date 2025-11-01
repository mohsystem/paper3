#include <iostream>
#include <cstring> // For std::memcpy
#include <new>     // For std::nothrow to prevent exceptions on failure

class Lse122 {
public:
    /**
     * Allocates two memory chunks using new[], copies content with std::memcpy,
     * and frees the memory using delete[].
     */
    static void demonstrateMemoryOperations() {
        // 1. Allocate two chunks of memory each of size 10.
        // Using char* as a char is typically one byte.
        // std::nothrow prevents an exception from being thrown on allocation failure.
        char* chunk1 = new (std::nothrow) char[10];
        char* chunk2 = new (std::nothrow) char[10];

        if (chunk1 == nullptr || chunk2 == nullptr) {
            std::cerr << "Memory allocation failed." << std::endl;
            // Clean up whichever chunk was successfully allocated before returning.
            delete[] chunk1;
            delete[] chunk2;
            return;
        }
        std::cout << "Allocated two chunks of 10 bytes." << std::endl;
        
        // For demonstration, fill the first chunk.
        for (int i = 0; i < 10; ++i) {
            chunk1[i] = static_cast<char>(i + 1);
        }

        // 2. Copy the contents of the first chunk into the second chunk.
        std::memcpy(chunk2, chunk1, 10);
        std::cout << "Copied contents from chunk1 to chunk2." << std::endl;

        // 3. Free the first chunk.
        delete[] chunk1;
        chunk1 = nullptr; // Good practice to prevent dangling pointers.
        std::cout << "Freed the first chunk." << std::endl;

        // 4. Free the second chunk.
        delete[] chunk2;
        chunk2 = nullptr; // Good practice.
        std::cout << "Freed the second chunk." << std::endl;
    }
};

int main() {
    std::cout << "--- Running C++ Test Cases ---" << std::endl;
    for (int i = 1; i <= 5; ++i) {
        std::cout << "\n--- Test Case " << i << " ---" << std::endl;
        Lse122::demonstrateMemoryOperations();
        std::cout << "--- End of Test Case " << i << " ---" << std::endl;
    }
    return 0;
}