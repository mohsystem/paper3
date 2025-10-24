
#include <iostream>
#include <string>
#include <stdexcept>

class Task2 {
public:
    static std::string createPhoneNumber(int* numbers, int size) {
        // Input validation
        if (numbers == nullptr) {
            throw std::invalid_argument("Input array cannot be null");
        }
        
        if (size != 10) {
            throw std::invalid_argument("Array must contain exactly 10 elements");
        }
        
        // Validate each element is between 0-9
        for (int i = 0; i < size; i++) {
            if (numbers[i] < 0 || numbers[i] > 9) {
                throw std::invalid_argument("All elements must be between 0 and 9");
            }
        }
        
        // Build phone number using safe string operations
        std::string phoneNumber = "(";
        phoneNumber += std::to_string(numbers[0]);
        phoneNumber += std::to_string(numbers[1]);
        phoneNumber += std::to_string(numbers[2]);
        phoneNumber += ") ";
        phoneNumber += std::to_string(numbers[3]);
        phoneNumber += std::to_string(numbers[4]);
        phoneNumber += std::to_string(numbers[5]);
        phoneNumber += "-";
        phoneNumber += std::to_string(numbers[6]);
        phoneNumber += std::to_string(numbers[7]);
        phoneNumber += std::to_string(numbers[8]);
        phoneNumber += std::to_string(numbers[9]);
        
        return phoneNumber;
    }
};

int main() {
    // Test case 1: Standard input
    int test1[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};
    std::cout << "Test 1: " << Task2::createPhoneNumber(test1, 10) << std::endl;
    
    // Test case 2: All zeros
    int test2[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::cout << "Test 2: " << Task2::createPhoneNumber(test2, 10) << std::endl;
    
    // Test case 3: All nines
    int test3[] = {9, 9, 9, 9, 9, 9, 9, 9, 9, 9};
    std::cout << "Test 3: " << Task2::createPhoneNumber(test3, 10) << std::endl;
    
    // Test case 4: Mixed numbers
    int test4[] = {5, 5, 5, 1, 2, 3, 4, 5, 6, 7};
    std::cout << "Test 4: " << Task2::createPhoneNumber(test4, 10) << std::endl;
    
    // Test case 5: Another mixed pattern
    int test5[] = {8, 0, 0, 5, 5, 5, 1, 2, 3, 4};
    std::cout << "Test 5: " << Task2::createPhoneNumber(test5, 10) << std::endl;
    
    return 0;
}
