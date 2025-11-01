
#include <iostream>
#include <cstring>

void fillBuffer(int bufferSize) {
    if (bufferSize <= 0) return;
    
    // Allocate buffer on stack
    char buffer[bufferSize];
    
    // Overwrite all values with 'A'
    memset(buffer, 'A', bufferSize);
}

int main() {
    // Test case 1: Small buffer
    fillBuffer(10);
    std::cout << "Test case 1: Buffer size 10 - Completed" << std::endl;
    
    // Test case 2: Medium buffer
    fillBuffer(100);
    std::cout << "Test case 2: Buffer size 100 - Completed" << std::endl;
    
    // Test case 3: Large buffer
    fillBuffer(1000);
    std::cout << "Test case 3: Buffer size 1000 - Completed" << std::endl;
    
    // Test case 4: Single element
    fillBuffer(1);
    std::cout << "Test case 4: Buffer size 1 - Completed" << std::endl;
    
    // Test case 5: Empty buffer
    fillBuffer(0);
    std::cout << "Test case 5: Buffer size 0 - Completed" << std::endl;
    
    return 0;
}
