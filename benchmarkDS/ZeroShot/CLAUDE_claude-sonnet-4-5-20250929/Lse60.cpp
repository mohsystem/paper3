
#include <iostream>
#include <vector>
#include <cstring>

void printBuffer(int size) {
    if (size <= 0) {
        std::cout << "Buffer size must be positive" << std::endl;
        return;
    }
    
    // Allocate buffer with specified size
    std::vector<char> buffer(size, 0);
    
    // Print buffer contents
    std::cout << "Buffer contents: [";
    for (int i = 0; i < size; i++) {
        std::cout << static_cast<int>(buffer[i]);
        if (i < size - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]" << std::endl;
}

int main(int argc, char* argv[]) {
    // Test cases
    std::cout << "Test Case 1:" << std::endl;
    printBuffer(5);
    
    std::cout << "\\nTest Case 2:" << std::endl;
    printBuffer(10);
    
    std::cout << "\\nTest Case 3:" << std::endl;
    printBuffer(1);
    
    std::cout << "\\nTest Case 4:" << std::endl;
    printBuffer(0);
    
    std::cout << "\\nTest Case 5:" << std::endl;
    printBuffer(20);
    
    // Command line argument handling
    if (argc > 1) {
        try {
            int size = std::stoi(argv[1]);
            std::cout << "\\nCommand line argument:" << std::endl;
            printBuffer(size);
        } catch (const std::exception& e) {
            std::cout << "Invalid argument. Please provide a valid integer." << std::endl;
        }
    }
    
    return 0;
}
