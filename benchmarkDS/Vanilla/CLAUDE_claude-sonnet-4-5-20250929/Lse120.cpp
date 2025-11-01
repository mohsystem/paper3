
#include <iostream>
#include <cstring>
#include <cstdlib>

void copyAndPrint(int size, const char* data) {
    // Allocate two chunks of memory
    char* chunk1 = (char*)malloc(size * sizeof(char));
    char* chunk2 = (char*)malloc(size * sizeof(char));
    
    // Initialize chunks with zeros
    memset(chunk1, 0, size);
    memset(chunk2, 0, size);
    
    // Copy data into first chunk
    strncpy(chunk1, data, size - 1);
    chunk1[size - 1] = '\\0';
    
    // Copy contents of first chunk into second chunk
    memcpy(chunk2, chunk1, size);
    
    // Print contents of second chunk
    std::cout << "Contents of second chunk: " << chunk2 << std::endl;
    
    // Free allocated memory
    free(chunk1);
    free(chunk2);
}

int main() {
    // Test case 1
    std::cout << "Test Case 1:" << std::endl;
    copyAndPrint(10, "Hello");
    
    // Test case 2
    std::cout << "\\nTest Case 2:" << std::endl;
    copyAndPrint(15, "World");
    
    // Test case 3
    std::cout << "\\nTest Case 3:" << std::endl;
    copyAndPrint(20, "Java Programming");
    
    // Test case 4
    std::cout << "\\nTest Case 4:" << std::endl;
    copyAndPrint(5, "Test");
    
    // Test case 5
    std::cout << "\\nTest Case 5:" << std::endl;
    copyAndPrint(30, "Memory Allocation Example");
    
    return 0;
}
