
#include <iostream>
#include <cstdlib>

void allocateAndFreeBuffer(int bufferSize, int iterations) {
    for (int i = 0; i < iterations; i++) {
        char* buffer = (char*)malloc(bufferSize);
        if (buffer == NULL) {
            std::cerr << "Memory allocation failed" << std::endl;
            return;
        }
        free(buffer);
    }
}

int main() {
    // Test case 1: Small buffer, few iterations
    std::cout << "Test 1: Buffer size 1024, 5 iterations" << std::endl;
    allocateAndFreeBuffer(1024, 5);
    
    // Test case 2: Medium buffer, moderate iterations
    std::cout << "Test 2: Buffer size 4096, 10 iterations" << std::endl;
    allocateAndFreeBuffer(4096, 10);
    
    // Test case 3: Large buffer, few iterations
    std::cout << "Test 3: Buffer size 1048576, 3 iterations" << std::endl;
    allocateAndFreeBuffer(1048576, 3);
    
    // Test case 4: Small buffer, many iterations
    std::cout << "Test 4: Buffer size 512, 20 iterations" << std::endl;
    allocateAndFreeBuffer(512, 20);
    
    // Test case 5: Very small buffer, moderate iterations
    std::cout << "Test 5: Buffer size 128, 15 iterations" << std::endl;
    allocateAndFreeBuffer(128, 15);
    
    std::cout << "All tests completed" << std::endl;
    
    return 0;
}
