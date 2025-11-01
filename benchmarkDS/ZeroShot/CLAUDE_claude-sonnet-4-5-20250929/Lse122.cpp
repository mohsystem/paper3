
#include <iostream>
#include <cstring>
#include <cstdlib>

void allocateAndCopy() {
    // Allocate first chunk of memory (size 10)
    char* chunk1 = (char*)malloc(10 * sizeof(char));
    
    if (chunk1 == nullptr) {
        std::cerr << "Memory allocation failed for chunk1" << std::endl;
        return;
    }
    
    // Initialize chunk1 with some data
    for (int i = 0; i < 10; i++) {
        chunk1[i] = i;
    }
    
    // Allocate second chunk of memory (size 10)
    char* chunk2 = (char*)malloc(10 * sizeof(char));
    
    if (chunk2 == nullptr) {
        std::cerr << "Memory allocation failed for chunk2" << std::endl;
        free(chunk1);
        return;
    }
    
    // Copy contents from chunk1 to chunk2
    memcpy(chunk2, chunk1, 10);
    
    // Free the first chunk
    free(chunk1);
    chunk1 = nullptr;
    
    // Free the second chunk
    free(chunk2);
    chunk2 = nullptr;
}

int main() {
    std::cout << "Test Case 1:" << std::endl;
    allocateAndCopy();
    
    std::cout << "Test Case 2:" << std::endl;
    allocateAndCopy();
    
    std::cout << "Test Case 3:" << std::endl;
    allocateAndCopy();
    
    std::cout << "Test Case 4:" << std::endl;
    allocateAndCopy();
    
    std::cout << "Test Case 5:" << std::endl;
    allocateAndCopy();
    
    std::cout << "All test cases completed successfully!" << std::endl;
    
    return 0;
}
