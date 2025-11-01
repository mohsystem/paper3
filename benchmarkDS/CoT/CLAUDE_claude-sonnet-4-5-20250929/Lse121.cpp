
#include <iostream>
#include <cstring>
#include <cstdlib>

void demonstrateMemoryOperations() {
    const char* initialData = "Hello, Secure World!";
    const size_t dataSize = strlen(initialData) + 1;
    
    // Allocate first chunk
    char* firstChunk = (char*)malloc(dataSize);
    if (firstChunk == NULL) {
        std::cerr << "Error: Memory allocation failed for first chunk" << std::endl;
        return;
    }
    
    // Copy initial data to first chunk
    strncpy(firstChunk, initialData, dataSize - 1);
    firstChunk[dataSize - 1] = '\\0';
    std::cout << "First chunk allocated: " << firstChunk << std::endl;
    
    // Allocate second chunk
    char* secondChunk = (char*)malloc(dataSize);
    if (secondChunk == NULL) {
        std::cerr << "Error: Memory allocation failed for second chunk" << std::endl;
        free(firstChunk);
        return;
    }
    
    // Copy contents from first chunk to second chunk
    strncpy(secondChunk, firstChunk, dataSize - 1);
    secondChunk[dataSize - 1] = '\\0';
    std::cout << "Second chunk after copy: " << secondChunk << std::endl;
    
    // Free first chunk
    free(firstChunk);
    firstChunk = NULL;
    std::cout << "First chunk freed" << std::endl;
    
    // Print second chunk after first is freed
    std::cout << "Second chunk after first freed: " << secondChunk << std::endl;
    
    // Free second chunk
    free(secondChunk);
    secondChunk = NULL;
    std::cout << "Second chunk freed" << std::endl;
}

int main() {
    std::cout << "=== Test Case 1: Basic operation ===" << std::endl;
    demonstrateMemoryOperations();
    
    std::cout << "\\n=== Test Case 2: Basic operation ===" << std::endl;
    demonstrateMemoryOperations();
    
    std::cout << "\\n=== Test Case 3: Basic operation ===" << std::endl;
    demonstrateMemoryOperations();
    
    std::cout << "\\n=== Test Case 4: Basic operation ===" << std::endl;
    demonstrateMemoryOperations();
    
    std::cout << "\\n=== Test Case 5: Basic operation ===" << std::endl;
    demonstrateMemoryOperations();
    
    return 0;
}
