
#include <iostream>
#include <cstring>
#include <cstdlib>

void allocateAndCopy() {
    // Allocate two chunks of memory, each of size 10
    char* chunk1 = (char*)malloc(10 * sizeof(char));
    char* chunk2 = (char*)malloc(10 * sizeof(char));
    
    if (chunk1 == nullptr || chunk2 == nullptr) {
        std::cerr << "Memory allocation failed!" << std::endl;
        return;
    }
    
    // Initialize first chunk with some data
    for (int i = 0; i < 10; i++) {
        chunk1[i] = i;
    }
    
    // Copy contents from chunk1 to chunk2
    memcpy(chunk2, chunk1, 10);
    
    // Free the first chunk
    free(chunk1);
    
    // Free the second chunk
    free(chunk2);
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
