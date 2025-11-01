
#include <iostream>
#include <cstring>
#include <cstdlib>

void memoryAllocationMethod() {
    // Allocate first buffer of 1024 bytes
    char* buf = (char*)malloc(1024 * sizeof(char));
    if (buf == NULL) {
        std::cerr << "Memory allocation failed for buf" << std::endl;
        return;
    }
    memset(buf, 0, 1024);
    
    // Read up to 100 characters from stdin into first buffer
    std::cin.getline(buf, 101);
    
    // Allocate second buffer of 1024 bytes
    char* buf1 = (char*)malloc(1024 * sizeof(char));
    if (buf1 == NULL) {
        std::cerr << "Memory allocation failed for buf1" << std::endl;
        free(buf);
        return;
    }
    memset(buf1, 0, 1024);
    
    // Read up to 100 characters from stdin into second buffer
    std::cin.getline(buf1, 101);
    
    std::cout << "First buffer (first 100 chars): " << buf << std::endl;
    std::cout << "Second buffer (first 100 chars): " << buf1 << std::endl;
    
    // Free allocated memory
    free(buf);
    free(buf1);
}

int main() {
    std::cout << "Test Case 1:" << std::endl;
    std::cout << "Enter first input: ";
    std::cout << "Enter second input: ";
    memoryAllocationMethod();
    
    std::cout << "\\nTest Case 2:" << std::endl;
    std::cout << "Enter first input: ";
    std::cout << "Enter second input: ";
    memoryAllocationMethod();
    
    std::cout << "\\nTest Case 3:" << std::endl;
    std::cout << "Enter first input: ";
    std::cout << "Enter second input: ";
    memoryAllocationMethod();
    
    std::cout << "\\nTest Case 4:" << std::endl;
    std::cout << "Enter first input: ";
    std::cout << "Enter second input: ";
    memoryAllocationMethod();
    
    std::cout << "\\nTest Case 5:" << std::endl;
    std::cout << "Enter first input: ";
    std::cout << "Enter second input: ";
    memoryAllocationMethod();
    
    return 0;
}
