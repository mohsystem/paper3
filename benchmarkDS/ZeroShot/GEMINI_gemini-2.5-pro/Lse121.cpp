#include <iostream>
#include <string>
#include <vector>
#include <cstring> // For memcpy
#include <new>     // For std::nothrow

// A helper function to print a memory chunk that may not be null-terminated.
void printChunk(const char* chunk, size_t size) {
    if (chunk == nullptr) {
        std::cout << "[null chunk]";
        return;
    }
    // std::cout.write is safe for printing raw buffers of a specific size.
    std::cout.write(chunk, size);
}

/**
 * @brief Demonstrates secure memory allocation, copying, and freeing in C++.
 *
 * @param content The string content to use for the memory chunks.
 */
void demonstrateMemoryOperations(const std::string& content) {
    std::cout << "---- Demonstrating for content: \"" << content << "\" ----" << std::endl;

    size_t size = content.length();
    
    // 1. Allocate two chunks of memory on the heap.
    // Using std::nothrow prevents an exception on failure and returns nullptr instead.
    char* chunk1 = new(std::nothrow) char[size];
    char* chunk2 = new(std::nothrow) char[size];
    
    // Secure coding: Always check if allocation was successful.
    if (chunk1 == nullptr || chunk2 == nullptr) {
        std::cerr << "Memory allocation failed." << std::endl;
        // Clean up any partial allocation before returning.
        delete[] chunk1;
        delete[] chunk2;
        return;
    }
    
    // Initialize chunk1 with the provided content.
    if (size > 0) {
        memcpy(chunk1, content.c_str(), size);
    }
    
    // 2. Copy the contents of the first chunk into the second chunk.
    // memcpy is used for raw memory copy. It's secure as long as the
    // source and destination buffers are valid and the size is correct.
    if (size > 0) {
        memcpy(chunk2, chunk1, size);
    }

    // 3. Print the contents of the second chunk.
    std::cout << "Contents of chunk2 before freeing chunk1: ";
    printChunk(chunk2, size);
    std::cout << std::endl;
    
    // 4. Free the memory allocated to the first chunk.
    delete[] chunk1;
    chunk1 = nullptr; // Good practice to nullify dangling pointers.
    
    std::cout << "chunk1 has been freed." << std::endl;
    
    // 5. Print the contents of the second chunk again.
    // The contents of chunk2 are independent and remain valid.
    std::cout << "Contents of chunk2 after freeing chunk1:  ";
    printChunk(chunk2, size);
    std::cout << std::endl;

    // 6. Free the memory allocated to the second chunk.
    delete[] chunk2;
    chunk2 = nullptr;
    std::cout << "chunk2 has been freed." << std::endl;
    std::cout << "------------------------------------------" << std::endl << std::endl;
}

int main() {
    // 5 test cases
    demonstrateMemoryOperations("Hello, Secure World!");
    demonstrateMemoryOperations(""); // Empty string
    std::string with_nulls = "A"; with_nulls += '\0'; with_nulls += "B"; with_nulls += '\0'; with_nulls += "C";
    demonstrateMemoryOperations(with_nulls); // String with embedded nulls
    demonstrateMemoryOperations("This is a much longer string to test a slightly larger allocation of memory.");
    demonstrateMemoryOperations("!@#$%^&*()_+-=[]{}|;':,./<>?");
    return 0;
}