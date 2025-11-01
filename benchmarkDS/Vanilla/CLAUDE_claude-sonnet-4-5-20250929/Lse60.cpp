
#include <iostream>
#include <cstdlib>
#include <cstring>

void processBuffer(int size) {
    char* buffer = new char[size];
    memset(buffer, 0, size);
    
    std::cout << "Buffer contents:" << std::endl;
    for (int i = 0; i < size; i++) {
        std::cout << (int)buffer[i] << " ";
        if ((i + 1) % 20 == 0) {
            std::cout << std::endl;
        }
    }
    std::cout << std::endl;
    
    delete[] buffer;
}

int main(int argc, char* argv[]) {
    // Test cases
    std::cout << "Test Case 1: Buffer size 10" << std::endl;
    processBuffer(10);
    
    std::cout << "\\nTest Case 2: Buffer size 25" << std::endl;
    processBuffer(25);
    
    std::cout << "\\nTest Case 3: Buffer size 50" << std::endl;
    processBuffer(50);
    
    std::cout << "\\nTest Case 4: Buffer size 5" << std::endl;
    processBuffer(5);
    
    std::cout << "\\nTest Case 5: Buffer size 100" << std::endl;
    processBuffer(100);
    
    // If command line argument is provided
    if (argc > 1) {
        int size = atoi(argv[1]);
        std::cout << "\\nCommand line argument: Buffer size " << size << std::endl;
        processBuffer(size);
    }
    
    return 0;
}
