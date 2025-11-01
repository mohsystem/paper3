#include <iostream>
#include <cstring> // For strlen and strcpy

class Lse121 {
public:
    static void demonstrateMemory(const char* data) {
        std::cout << "--- Running test with data: \"" << data << "\" ---" << std::endl;

        size_t size = strlen(data) + 1; // +1 for the null terminator

        // 1. Allocate two chunks of memory.
        char* chunk1 = new char[size];
        char* chunk2 = new char[size];
        std::cout << "Step 1: Two chunks allocated." << std::endl;

        // 2. Copy the contents of the first chunk into the second chunk.
        strcpy(chunk1, data);
        strcpy(chunk2, chunk1);
        std::cout << "Step 2: Contents of chunk1 copied to chunk2." << std::endl;

        // 3. Print the contents of the second chunk.
        std::cout << "Step 3: Contents of chunk2 after copy: " << chunk2 << std::endl;

        // 4. Free the memory allocated to the first chunk.
        delete[] chunk1;
        chunk1 = nullptr; // Good practice to nullify dangling pointers
        std::cout << "Step 4: chunk1 has been freed." << std::endl;

        // 5. Print the contents of the second chunk again.
        // The contents should be unchanged as chunk2 is a separate memory block.
        std::cout << "Step 5: Contents of chunk2 after freeing chunk1: " << chunk2 << std::endl;
        
        // 6. Free the memory allocated to the second chunk.
        delete[] chunk2;
        chunk2 = nullptr;
        std::cout << "Step 6: chunk2 has been freed." << std::endl;
        std::cout << "--- Test finished ---" << std::endl << std::endl;
    }
};

int main() {
    // 5 test cases
    Lse121::demonstrateMemory("Hello World");
    Lse121::demonstrateMemory("CPP Test Case");
    Lse121::demonstrateMemory("12345");
    Lse121::demonstrateMemory("Another piece of data.");
    Lse121::demonstrateMemory(""); // Empty string test
    return 0;
}