
#include <iostream>
#include <cstring>
#include <cstdlib>

void copyAndPrint(const char* source) {
    if (source == nullptr) {
        std::cout << "Source is null" << std::endl;
        return;
    }
    
    // Get length of source string
    size_t length = strlen(source);
    
    // Allocate first chunk of memory
    char* firstChunk = (char*)malloc(length + 1);
    if (firstChunk == nullptr) {
        std::cout << "Memory allocation failed" << std::endl;
        return;
    }
    
    // Copy source to first chunk
    strncpy(firstChunk, source, length);
    firstChunk[length] = '\\0';
    
    // Allocate second chunk of memory
    char* secondChunk = (char*)malloc(length + 1);
    if (secondChunk == nullptr) {
        std::cout << "Memory allocation failed" << std::endl;
        free(firstChunk);
        return;
    }
    
    // Copy first chunk to second chunk
    strncpy(secondChunk, firstChunk, length);
    secondChunk[length] = '\\0';
    
    // Print contents of second chunk
    std::cout << secondChunk << std::endl;
    
    // Free allocated memory
    free(firstChunk);
    free(secondChunk);
}

int main() {
    // Test case 1: Normal string
    std::cout << "Test 1:" << std::endl;
    copyAndPrint("Hello, World!");
    
    // Test case 2: Empty string
    std::cout << "\\nTest 2:" << std::endl;
    copyAndPrint("");
    
    // Test case 3: String with special characters
    std::cout << "\\nTest 3:" << std::endl;
    copyAndPrint("Special @#$% Characters!");
    
    // Test case 4: Null input
    std::cout << "\\nTest 4:" << std::endl;
    copyAndPrint(nullptr);
    
    // Test case 5: Long string
    std::cout << "\\nTest 5:" << std::endl;
    copyAndPrint("This is a longer string with multiple words and spaces");
    
    return 0;
}
