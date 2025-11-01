
#include <iostream>
#include <cstring>
#include <stdexcept>

void copyBytes(unsigned char* destination, const unsigned char* source, size_t numBytes) {
    if (destination == nullptr || source == nullptr) {
        throw std::invalid_argument("Destination and source cannot be null");
    }
    
    if (numBytes == 0) {
        return;
    }
    
    std::memcpy(destination, source, numBytes);
}

int main() {
    // Test case 1: Normal copy
    unsigned char src1[] = {1, 2, 3, 4, 5};
    unsigned char dst1[5] = {0};
    copyBytes(dst1, src1, 5);
    std::cout << "Test 1: ";
    for (int i = 0; i < 5; i++) std::cout << (int)dst1[i] << " ";
    std::cout << std::endl;
    
    // Test case 2: Partial copy
    unsigned char src2[] = {10, 20, 30, 40, 50};
    unsigned char dst2[5] = {0};
    copyBytes(dst2, src2, 3);
    std::cout << "Test 2: ";
    for (int i = 0; i < 5; i++) std::cout << (int)dst2[i] << " ";
    std::cout << std::endl;
    
    // Test case 3: Copy 0 bytes
    unsigned char src3[] = {1, 2, 3};
    unsigned char dst3[3] = {0};
    copyBytes(dst3, src3, 0);
    std::cout << "Test 3: ";
    for (int i = 0; i < 3; i++) std::cout << (int)dst3[i] << " ";
    std::cout << std::endl;
    
    // Test case 4: Single byte copy
    unsigned char src4[] = {99};
    unsigned char dst4[1] = {0};
    copyBytes(dst4, src4, 1);
    std::cout << "Test 4: ";
    for (int i = 0; i < 1; i++) std::cout << (int)dst4[i] << " ";
    std::cout << std::endl;
    
    // Test case 5: Larger array
    unsigned char src5[] = {11, 22, 33, 44, 55, 66, 77, 88};
    unsigned char dst5[8] = {0};
    copyBytes(dst5, src5, 8);
    std::cout << "Test 5: ";
    for (int i = 0; i < 8; i++) std::cout << (int)dst5[i] << " ";
    std::cout << std::endl;
    
    return 0;
}
