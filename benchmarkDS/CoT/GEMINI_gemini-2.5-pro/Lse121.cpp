#include <iostream>
#include <cstring> // For strlen and memcpy
#include <vector>   // For test cases

class Lse121 {
public:
    static void demonstrateMemoryManagement(const char* initialData) {
        std::cout << "Processing data: \"" << initialData << "\"" << std::endl;

        size_t dataSize = strlen(initialData) + 1; // +1 for the null terminator

        // 1. Allocate two chunks of memory.
        char* chunk1 = new (std::nothrow) char[dataSize];
        char* chunk2 = new (std::nothrow) char[dataSize];

        if (chunk1 == nullptr || chunk2 == nullptr) {
            std::cerr << "Error: Memory allocation failed." << std::endl;
            delete[] chunk1; // Safely delete even if it's nullptr
            delete[] chunk2;
            return;
        }
        
        // Initialize the first chunk
        memcpy(chunk1, initialData, dataSize);
        std::cout << "Step 1 & 2: Allocated two chunks and initialized first chunk." << std::endl;

        // 3. Copy the contents of the first chunk into the second chunk.
        memcpy(chunk2, chunk1, dataSize);
        std::cout << "Step 3: Copied chunk1 to chunk2." << std::endl;

        // 4. Print the contents of the second chunk.
        std::cout << "Step 4: Contents of chunk2 after copy: " << chunk2 << std::endl;

        // 5. Free the memory allocated to the first chunk.
        std::cout << "Step 5: Freeing chunk1..." << std::endl;
        delete[] chunk1;
        chunk1 = nullptr; // Good practice to avoid dangling pointers

        // 6. Print the contents of the second chunk again.
        // The content should remain as chunk2 is an independent copy.
        std::cout << "Step 6: Contents of chunk2 after freeing chunk1: " << chunk2 << std::endl;

        // 7. Free the memory allocated to the second chunk.
        std::cout << "Step 7: Freeing chunk2..." << std::endl;
        delete[] chunk2;
        chunk2 = nullptr;

        std::cout << "----------------------------------------" << std::endl;
    }
};

int main() {
    // 5 test cases
    Lse121::demonstrateMemoryManagement("Test Case 1: Hello World");
    Lse121::demonstrateMemoryManagement("Test Case 2: A simple string");
    Lse121::demonstrateMemoryManagement("Test Case 3: Data");
    Lse121::demonstrateMemoryManagement("Test Case 4: Another example with numbers 123");
    Lse121::demonstrateMemoryManagement("Test Case 5: Final test!");

    return 0;
}