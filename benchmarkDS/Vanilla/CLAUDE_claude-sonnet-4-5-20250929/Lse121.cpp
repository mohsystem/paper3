
#include <iostream>
#include <cstring>
#include <cstdlib>

class Lse121 {
public:
    static void demonstrateMemoryCopy() {
        // Allocate two chunks of memory
        int size = 10;
        char* chunk1 = (char*)malloc(size * sizeof(char));
        char* chunk2 = (char*)malloc(size * sizeof(char));
        
        // Fill first chunk with some data
        for (int i = 0; i < size; i++) {
            chunk1[i] = 'A' + i;
        }
        
        // Copy contents of first chunk into second chunk
        memcpy(chunk2, chunk1, size);
        
        // Print contents of second chunk
        std::cout << "Contents of chunk2 after copy:" << std::endl;
        for (int i = 0; i < size; i++) {
            std::cout << chunk2[i] << " ";
        }
        std::cout << std::endl;
        
        // Free memory allocated to first chunk
        free(chunk1);
        chunk1 = nullptr;
        
        // Print contents of second chunk again
        std::cout << "Contents of chunk2 after freeing chunk1:" << std::endl;
        for (int i = 0; i < size; i++) {
            std::cout << chunk2[i] << " ";
        }
        std::cout << std::endl;
        
        // Free memory allocated to second chunk
        free(chunk2);
        chunk2 = nullptr;
        
        std::cout << "Both chunks freed." << std::endl;
    }
};

int main() {
    std::cout << "Test Case 1:" << std::endl;
    Lse121::demonstrateMemoryCopy();
    
    std::cout << "\\nTest Case 2:" << std::endl;
    Lse121::demonstrateMemoryCopy();
    
    std::cout << "\\nTest Case 3:" << std::endl;
    Lse121::demonstrateMemoryCopy();
    
    std::cout << "\\nTest Case 4:" << std::endl;
    Lse121::demonstrateMemoryCopy();
    
    std::cout << "\\nTest Case 5:" << std::endl;
    Lse121::demonstrateMemoryCopy();
    
    return 0;
}
