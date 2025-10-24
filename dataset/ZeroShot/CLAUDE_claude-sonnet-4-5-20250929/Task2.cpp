
#include <iostream>
#include <string>
#include <stdexcept>

std::string createPhoneNumber(int numbers[], int size) {
    if (numbers == nullptr || size != 10) {
        throw std::invalid_argument("Array must contain exactly 10 integers");
    }
    
    for (int i = 0; i < size; i++) {
        if (numbers[i] < 0 || numbers[i] > 9) {
            throw std::invalid_argument("All numbers must be between 0 and 9");
        }
    }
    
    char buffer[15];
    snprintf(buffer, sizeof(buffer), "(%d%d%d) %d%d%d-%d%d%d%d",
        numbers[0], numbers[1], numbers[2],
        numbers[3], numbers[4], numbers[5],
        numbers[6], numbers[7], numbers[8], numbers[9]);
    
    return std::string(buffer);
}

int main() {
    // Test case 1
    int test1[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};
    std::cout << createPhoneNumber(test1, 10) << std::endl;
    
    // Test case 2
    int test2[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::cout << createPhoneNumber(test2, 10) << std::endl;
    
    // Test case 3
    int test3[] = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
    std::cout << createPhoneNumber(test3, 10) << std::endl;
    
    // Test case 4
    int test4[] = {5, 5, 5, 1, 2, 3, 4, 5, 6, 7};
    std::cout << createPhoneNumber(test4, 10) << std::endl;
    
    // Test case 5
    int test5[] = {1, 1, 1, 2, 2, 2, 3, 3, 3, 4};
    std::cout << createPhoneNumber(test5, 10) << std::endl;
    
    return 0;
}
