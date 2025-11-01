#include <iostream>
#include <cstring> // For memcpy, memcmp
#include <new>     // For std::nothrow

/**
 * @brief This function demonstrates manual memory management in C++.
 * It allocates two memory chunks, copies data, and then frees them.
 * Note: For modern C++, prefer using RAII containers like std::vector or smart
 * pointers (std::unique_ptr, std::shared_ptr) to manage memory automatically
 * and safely.
 */
void manageMemory() {
    const size_t CHUNK_SIZE = 10;
    char* chunk1 = nullptr;
    char* chunk2 = nullptr;

    // 1. Allocate two chunks of memory each of size 10.
    // Use std::nothrow to prevent exceptions on failure; it returns nullptr instead.
    chunk1 = new (std::nothrow) char[CHUNK_SIZE];
    chunk2 = new (std::nothrow) char[CHUNK_SIZE];

    if (!chunk1 || !chunk2) {
        std::cerr << "Error: Memory allocation failed." << std::endl;
        delete[] chunk1; // delete[] on nullptr is safe.
        delete[] chunk2;
        return;
    }
    std::cout << "Allocated two chunks of size " << CHUNK_SIZE << std::endl;

    // Initialize chunk1 with some data for demonstration
    for (size_t i = 0; i < CHUNK_SIZE; ++i) {
        chunk1[i] = static_cast<char>(i + 1);
    }
    std::cout << "Initialized chunk1." << std::endl;

    // 2. Copy the contents of the first chunk into the second chunk.
    // memcpy is safe here because both buffers are known to be of CHUNK_SIZE.
    memcpy(chunk2, chunk1, CHUNK_SIZE);
    std::cout << "Copied contents of chunk1 to chunk2." << std::endl;

    // Verification step
    if (memcmp(chunk1, chunk2, CHUNK_SIZE) == 0) {
        std::cout << "Verification successful: chunk2 is a copy of chunk1." << std::endl;
    } else {
        std::cout << "Verification failed: copy was not successful." << std::endl;
    }

    // 3. Free the first chunk.
    // NOTE: If the data were sensitive, it should be securely zeroed out before freeing
    // using a function like memset_s or a custom volatile-based wipe.
    delete[] chunk1;
    chunk1 = nullptr; // Best practice: prevent use-after-free vulnerabilities.
    std::cout << "Freed chunk1." << std::endl;
    
    // 4. Free the second chunk.
    delete[] chunk2;
    chunk2 = nullptr; // Best practice: prevent use-after-free vulnerabilities.
    std::cout << "Freed chunk2." << std::endl;
    std::cout << "Memory management simulation complete." << std::endl;
}

int main() {
    for (int i = 0; i < 5; ++i) {
        std::cout << "\n--- Test Case " << (i + 1) << " ---" << std::endl;
        manageMemory();
    }
    return 0;
}