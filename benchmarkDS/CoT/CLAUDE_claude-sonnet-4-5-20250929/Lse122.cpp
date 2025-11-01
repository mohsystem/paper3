
#include <iostream>
#include <cstring>
#include <cstdlib>

void memoryOperation() {
    // Allocate two chunks of memory each of size 10
    char* chunk1 = static_cast<char*>(malloc(10));
    char* chunk2 = static_cast<char*>(malloc(10));
    
    // Check if allocation was successful
    if (chunk1 == nullptr || chunk2 == nullptr) {
        std::cerr << "Memory allocation failed" << std::endl;
        free(chunk1);
        free(chunk2);
        return;
    }
    
    // Initialize first chunk with some data
    for (int i = 0; i < 10; i++) {
        chunk1[i] = static_cast<char>(i);
    }
    
    // Copy contents from chunk1 to chunk2 safely
    memcpy(chunk2, chunk1, 10);
    
    // Free the first chunk
    free(chunk1);
    chunk1 = nullptr;
    
    // Free the second chunk
    free(chunk2);
    chunk2 = nullptr;
}

int main() {
    std::cout << "Test Case 1: Basic memory operation" << std::endl;
    memoryOperation();
    
    std::cout << "Test Case 2: Multiple operations" << std::endl;
    memoryOperation();
    
    std::cout << "Test Case 3: Sequential operations" << std::endl;
    memoryOperation();
    
    std::cout << "Test Case 4: Memory stress test" << std::endl;
    memoryOperation();
    
    std::cout << "Test Case 5: Final operation" << std::endl;
    memoryOperation();
    
    std::cout << "All test cases completed successfully" << std::endl;
    
    return 0;
}
