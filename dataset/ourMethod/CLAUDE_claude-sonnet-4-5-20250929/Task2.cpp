
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

// Function to create phone number from array of 10 integers
// Security: Validates input array size and integer ranges (0-9)
// Returns formatted phone number string: (###) ###-####
std::string createPhoneNumber(const std::vector<int>& numbers) {
    // Validate input size - must be exactly 10 integers
    if (numbers.size() != 10) {
        throw std::invalid_argument("Input must contain exactly 10 integers");
    }
    
    // Validate each integer is in range 0-9
    for (size_t i = 0; i < numbers.size(); ++i) {
        if (numbers[i] < 0 || numbers[i] > 9) {
            throw std::invalid_argument("All integers must be between 0 and 9");
        }
    }
    
    // Construct phone number string using safe string operations
    // Format: (###) ###-####
    std::string result = "(";
    result += std::to_string(numbers[0]);
    result += std::to_string(numbers[1]);
    result += std::to_string(numbers[2]);
    result += ") ";
    result += std::to_string(numbers[3]);
    result += std::to_string(numbers[4]);
    result += std::to_string(numbers[5]);
    result += "-";
    result += std::to_string(numbers[6]);
    result += std::to_string(numbers[7]);
    result += std::to_string(numbers[8]);
    result += std::to_string(numbers[9]);
    
    return result;
}

int main() {
    // Test case 1: Standard example
    try {
        std::vector<int> test1 = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};
        std::cout << "Test 1: " << createPhoneNumber(test1) << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 1 Error: " << e.what() << std::endl;
    }
    
    // Test case 2: All zeros
    try {
        std::vector<int> test2 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        std::cout << "Test 2: " << createPhoneNumber(test2) << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 2 Error: " << e.what() << std::endl;
    }
    
    // Test case 3: All nines
    try {
        std::vector<int> test3 = {9, 9, 9, 9, 9, 9, 9, 9, 9, 9};
        std::cout << "Test 3: " << createPhoneNumber(test3) << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 3 Error: " << e.what() << std::endl;
    }
    
    // Test case 4: Invalid - wrong size
    try {
        std::vector<int> test4 = {1, 2, 3, 4, 5};
        std::cout << "Test 4: " << createPhoneNumber(test4) << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 4 Error: " << e.what() << std::endl;
    }
    
    // Test case 5: Invalid - out of range value
    try {
        std::vector<int> test5 = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        std::cout << "Test 5: " << createPhoneNumber(test5) << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 5 Error: " << e.what() << std::endl;
    }
    
    return 0;
}
