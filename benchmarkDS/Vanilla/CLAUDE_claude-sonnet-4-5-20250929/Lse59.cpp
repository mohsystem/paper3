
#include <iostream>
#include <cstring>

void allocateAndFillBuffer(int bufferSize) {
    char buffer[bufferSize];
    memset(buffer, 'A', bufferSize);
}

int main() {
    // Test case 1
    allocateAndFillBuffer(10);
    std::cout << "Test case 1: Buffer size 10 - Completed" << std::endl;
    
    // Test case 2
    allocateAndFillBuffer(50);
    std::cout << "Test case 2: Buffer size 50 - Completed" << std::endl;
    
    // Test case 3
    allocateAndFillBuffer(100);
    std::cout << "Test case 3: Buffer size 100 - Completed" << std::endl;
    
    // Test case 4
    allocateAndFillBuffer(1);
    std::cout << "Test case 4: Buffer size 1 - Completed" << std::endl;
    
    // Test case 5
    allocateAndFillBuffer(500);
    std::cout << "Test case 5: Buffer size 500 - Completed" << std::endl;
    
    return 0;
}
