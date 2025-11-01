
#include <iostream>
#include <cstring>
#include <cstdlib>

class Lse121 {
public:
    static void demonstrateMemoryCopy() {
        // Allocate two chunks of memory
        const int size = 100;
        char* firstChunk = (char*)malloc(size * sizeof(char));
        char* secondChunk = (char*)malloc(size * sizeof(char));
        
        if (firstChunk == nullptr || secondChunk == nullptr) {
            std::cerr << "Memory allocation failed!" << std::endl;
            if (firstChunk) free(firstChunk);
            if (secondChunk) free(secondChunk);
            return;
        }
        
        // Initialize memory to zero
        memset(firstChunk, 0, size);
        memset(secondChunk, 0, size);
        
        // Fill first chunk with some data
        const char* data = "Hello, this is test data for memory operation!";
        strncpy(firstChunk, data, size - 1);
        firstChunk[size - 1] = '\\0';
        
        // Copy contents of first chunk into second chunk
        memcpy(secondChunk, firstChunk, size);
        
        // Print contents of second chunk
        std::cout << "Contents of second chunk after copy:" << std::endl;
        std::cout << secondChunk << std::endl;
        
        // Free memory of first chunk
        free(firstChunk);
        firstChunk = nullptr;
        
        // Print contents of second chunk again
        std::cout << "\\nContents of second chunk after freeing first chunk:" << std::endl;
        std::cout << secondChunk << std::endl;
        
        // Free memory of second chunk
        free(secondChunk);
        secondChunk = nullptr;
        
        std::cout << "\\nMemory operations completed successfully." << std::endl;
    }
};

int main() {
    std::cout << "Test Case 1:" << std::endl;
    Lse121::demonstrateMemoryCopy();
    
    std::cout << "\\n" << std::string(50, '=') << "\\n" << std::endl;
    
    std::cout << "Test Case 2:" << std::endl;
    Lse121::demonstrateMemoryCopy();
    
    std::cout << "\\n" << std::string(50, '=') << "\\n" << std::endl;
    
    std::cout << "Test Case 3:" << std::endl;
    Lse121::demonstrateMemoryCopy();
    
    std::cout << "\\n" << std::string(50, '=') << "\\n" << std::endl;
    
    std::cout << "Test Case 4:" << std::endl;
    Lse121::demonstrateMemoryCopy();
    
    std::cout << "\\n" << std::string(50, '=') << "\\n" << std::endl;
    
    std::cout << "Test Case 5:" << std::endl;
    Lse121::demonstrateMemoryCopy();
    
    return 0;
}
