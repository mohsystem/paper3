#include <iostream>
#include <algorithm> // For std::copy
#include <cstring>   // For std::memcpy
#include <new>       // For std::bad_alloc

class Lse122 {
public:
    static void secureMemoryOperations() {
        std::cout << "--- C++: Starting memory operations ---" << std::endl;

        char* chunk1 = nullptr;
        char* chunk2 = nullptr;

        try {
            // 1. Allocate two chunks of memory each of size 10.
            chunk1 = new char[10];
            std::cout << "Step 1: Allocated chunk1 of size 10." << std::endl;
            
            chunk2 = new char[10];
            std::cout << "Step 2: Allocated chunk2 of size 10." << std::endl;
            
            // Initialize chunk1 with some data for demonstration.
            const char initial_data[10] = "123456789"; // 9 chars + null terminator
            std::memcpy(chunk1, initial_data, 10);
            std::cout << "Initialized chunk1." << std::endl;

            // 2. Copy the contents of the first chunk into the second chunk.
            // std::copy is a safe, idiomatic C++ way to copy ranges.
            std::copy(chunk1, chunk1 + 10, chunk2);
            std::cout << "Step 3: Copied contents from chunk1 to chunk2." << std::endl;

        } catch (const std::bad_alloc& e) {
            std::cerr << "Memory allocation failed: " << e.what() << std::endl;
            // Free any memory that was successfully allocated before the failure.
            delete[] chunk1; // It is safe to delete a nullptr.
            delete[] chunk2;
            return;
        }

        // 3. Free the first chunk.
        // Use delete[] for memory allocated with new[].
        delete[] chunk1;
        // Setting the pointer to nullptr is a security best practice to prevent
        // use-after-free vulnerabilities.
        chunk1 = nullptr;
        std::cout << "Step 4: Freed chunk1 and set pointer to nullptr." << std::endl;

        // 4. Free the second chunk.
        delete[] chunk2;
        chunk2 = nullptr;
        std::cout << "Step 5: Freed chunk2 and set pointer to nullptr." << std::endl;
        
        std::cout << "--- C++: Finished memory operations ---" << std::endl;
    }
};

int main() {
    for (int i = 0; i < 5; ++i) {
        std::cout << "\nTest Case " << (i + 1) << std::endl;
        Lse122::secureMemoryOperations();
    }
    return 0;
}