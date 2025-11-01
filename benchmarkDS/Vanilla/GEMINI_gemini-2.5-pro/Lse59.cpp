#include <iostream>
#include <cstring>

void overwriteBuffer(const int buffer_size) {
    if (buffer_size < 0) {
        std::cout << "overwriteBuffer(size=" << buffer_size << "): Invalid size" << std::endl;
        return;
    }
    char buffer[buffer_size];
    if (buffer_size > 0) {
        memset(buffer, 'A', buffer_size);
    }
    
    std::cout << "overwriteBuffer(size=" << buffer_size << "): ";
    for (int i = 0; i < buffer_size; ++i) {
        std::cout << buffer[i];
    }
    std::cout << std::endl;
}

int main() {
    // Test Case 1
    overwriteBuffer(10);
    
    // Test Case 2
    overwriteBuffer(5);
    
    // Test Case 3
    overwriteBuffer(20);
    
    // Test Case 4
    overwriteBuffer(1);
    
    // Test Case 5
    overwriteBuffer(0);
    
    return 0;
}