#include <iostream>
#include <cstring> // For std::memcpy, std::memset
#include <vector>
#include <iomanip>  // For std::hex, std::setw, std::setfill
#include <new>      // For std::nothrow

// Helper function to print a memory chunk in a readable hex format.
void print_chunk(const char* name, const char* chunk, size_t size) {
    if (chunk == nullptr) {
        std::cout << "   " << name << " is nullptr" << std::endl;
        return;
    }
    std::cout << "   Content of " << name << ": [ ";
    for (size_t i = 0; i < size; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(static_cast<unsigned char>(chunk[i])) << " ";
    }
    std::cout << "]" << std::dec << std::endl;
}

void manageMemory(int testCaseNumber) {
    const size_t SIZE = 10;
    std::cout << "--- Starting Memory Management Demo ---" << std::endl;

    // 1. Allocate two chunks of memory each of size 10.
    // Use `new (std::nothrow)` which returns nullptr on failure instead of throwing an exception.
    char* chunk1 = new (std::nothrow) char[SIZE];
    if (chunk1 == nullptr) {
        std::cerr << "Error: Failed to allocate chunk1." << std::endl;
        return;
    }
    char* chunk2 = new (std::nothrow) char[SIZE];
    if (chunk2 == nullptr) {
        std::cerr << "Error: Failed to allocate chunk2." << std::endl;
        delete[] chunk1; // Clean up previously allocated memory before returning.
        return;
    }
    std::cout << "1. Allocated two memory chunks of size " << SIZE << std::endl;

    // Initialize the first chunk with some data.
    // `snprintf` is used for safe string formatting to avoid buffer overflows.
    char testData[SIZE];
    snprintf(testData, SIZE, "Test %d", testCaseNumber);
    std::memcpy(chunk1, testData, SIZE);
    
    print_chunk("chunk1", chunk1, SIZE);
    std::memset(chunk2, 0, SIZE); // Zero out chunk2 to show it's initially empty.
    print_chunk("chunk2 (before copy)", chunk2, SIZE);

    // 2. Copy the contents of the first chunk into the second chunk.
    // `std::memcpy` is used for a raw memory copy. The size is known, so it's safe here.
    std::memcpy(chunk2, chunk1, SIZE);
    std::cout << "2. Copied chunk1 to chunk2." << std::endl;
    print_chunk("chunk2 (after copy)", chunk2, SIZE);

    // 3. Free the first chunk.
    delete[] chunk1;
    chunk1 = nullptr; // Security best practice: set pointer to nullptr after freeing to prevent use-after-free.
    std::cout << "3. Freed first chunk." << std::endl;

    // 4. Free the second chunk.
    delete[] chunk2;
    chunk2 = nullptr; // Prevent use-after-free.
    std::cout << "4. Freed second chunk." << std::endl;

    std::cout << "--- Demo Finished ---" << std::endl << std::endl;
}

int main() {
    std::cout << ">>> Running 5 Test Cases for Memory Management in C++ <<<" << std::endl;
    for (int i = 1; i <= 5; ++i) {
        std::cout << ">>> Test Case #" << i << " <<<" << std::endl;
        manageMemory(i);
    }
    return 0;
}