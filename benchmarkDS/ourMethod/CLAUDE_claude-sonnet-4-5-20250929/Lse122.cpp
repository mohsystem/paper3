
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <new>

// Function to allocate, copy, and free memory chunks
void allocateCopyAndFree() {
    unsigned char* chunk1 = nullptr;
    unsigned char* chunk2 = nullptr;
    const size_t CHUNK_SIZE = 10;
    
    try {
        // Allocate first chunk of 10 bytes with error checking
        chunk1 = static_cast<unsigned char*>(std::malloc(CHUNK_SIZE));
        if (chunk1 == nullptr) {
            throw std::bad_alloc();
        }
        
        // Initialize chunk1 to zero for safety
        std::memset(chunk1, 0, CHUNK_SIZE);
        
        // Fill chunk1 with sample data for testing
        for (size_t i = 0; i < CHUNK_SIZE; ++i) {
            chunk1[i] = static_cast<unsigned char>(i);
        }
        
        // Allocate second chunk of 10 bytes with error checking
        chunk2 = static_cast<unsigned char*>(std::malloc(CHUNK_SIZE));
        if (chunk2 == nullptr) {
            // Clean up chunk1 before throwing
            std::memset(chunk1, 0, CHUNK_SIZE);
            std::free(chunk1);
            throw std::bad_alloc();
        }
        
        // Initialize chunk2 to zero for safety
        std::memset(chunk2, 0, CHUNK_SIZE);
        
        // Copy contents from chunk1 to chunk2 - memcpy is safe here as no overlap
        std::memcpy(chunk2, chunk1, CHUNK_SIZE);
        
        // Clear sensitive data from chunk1 before freeing
        // Using volatile to prevent compiler optimization
        volatile unsigned char* vptr = chunk1;
        for (size_t i = 0; i < CHUNK_SIZE; ++i) {
            vptr[i] = 0;
        }
        std::free(chunk1);
        chunk1 = nullptr;
        
        // Clear sensitive data from chunk2 before freeing
        vptr = chunk2;
        for (size_t i = 0; i < CHUNK_SIZE; ++i) {
            vptr[i] = 0;
        }
        std::free(chunk2);
        chunk2 = nullptr;
        
    } catch (const std::bad_alloc& e) {
        std::cerr << "Memory allocation failed: " << e.what() << std::endl;
        // Cleanup already handled in error paths above
    } catch (...) {
        std::cerr << "Unexpected error occurred" << std::endl;
        // Ensure cleanup on unexpected error
        if (chunk1 != nullptr) {
            volatile unsigned char* vptr = chunk1;
            for (size_t i = 0; i < CHUNK_SIZE; ++i) {
                vptr[i] = 0;
            }
            std::free(chunk1);
        }
        if (chunk2 != nullptr) {
            volatile unsigned char* vptr = chunk2;
            for (size_t i = 0; i < CHUNK_SIZE; ++i) {
                vptr[i] = 0;
            }
            std::free(chunk2);
        }
    }
}

int main() {
    // Test case 1: Normal operation
    std::cout << "Test 1: Normal allocation, copy, and free" << std::endl;
    allocateCopyAndFree();
    
    // Test case 2: Repeated operations
    std::cout << "Test 2: Repeated operations" << std::endl;
    allocateCopyAndFree();
    
    // Test case 3: Multiple sequential calls
    std::cout << "Test 3: Multiple sequential calls" << std::endl;
    for (int i = 0; i < 3; ++i) {
        allocateCopyAndFree();
    }
    
    // Test case 4: Single operation
    std::cout << "Test 4: Single operation" << std::endl;
    allocateCopyAndFree();
    
    // Test case 5: Final operation
    std::cout << "Test 5: Final operation" << std::endl;
    allocateCopyAndFree();
    
    std::cout << "All tests completed successfully" << std::endl;
    
    return 0;
}
