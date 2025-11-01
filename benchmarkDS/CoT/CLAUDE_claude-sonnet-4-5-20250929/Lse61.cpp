
#include <iostream>
#include <cstdlib>

void allocateAndFreeBuffer(int bufferSize) {
    if (bufferSize <= 0) {
        std::cout << "Invalid buffer size" << std::endl;
        return;
    }
    
    // Allocate buffer
    char* buffer = (char*)malloc(bufferSize);
    if (buffer == nullptr) {
        std::cout << "Memory allocation failed" << std::endl;
        return;
    }
    std::cout << "Buffer of size " << bufferSize << " allocated" << std::endl;
    
    // Use the buffer (example)
    for (int i = 0; i < (bufferSize < 10 ? bufferSize : 10); i++) {
        buffer[i] = i;
    }
    
    // Free buffer ONCE (not in a loop to prevent double-free vulnerability)
    free(buffer);
    buffer = nullptr;  // Set to nullptr to prevent use-after-free
    std::cout << "Buffer freed safely" << std::endl;
}

int main() {
    std::cout << "Test Case 1: Small buffer" << std::endl;
    allocateAndFreeBuffer(10);
    
    std::cout << "\\nTest Case 2: Medium buffer" << std::endl;
    allocateAndFreeBuffer(100);
    
    std::cout << "\\nTest Case 3: Large buffer" << std::endl;
    allocateAndFreeBuffer(1000);
    
    std::cout << "\\nTest Case 4: Invalid buffer (zero)" << std::endl;
    allocateAndFreeBuffer(0);
    
    std::cout << "\\nTest Case 5: Invalid buffer (negative)" << std::endl;
    allocateAndFreeBuffer(-10);
    
    return 0;
}
