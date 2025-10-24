
#include <iostream>
#include <string>
#include <random>
#include <stdexcept>

std::string generateRandomString(int length) {
    if (length < 0) {
        throw std::invalid_argument("Length cannot be negative");
    }
    if (length == 0) {
        return "";
    }
    if (length > 1000000) {
        throw std::invalid_argument("Length too large to prevent resource exhaustion");
    }
    
    const std::string ASCII_LETTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<int> distribution(0, ASCII_LETTERS.length() - 1);
    
    std::string result;
    result.reserve(length);
    
    for (int i = 0; i < length; i++) {
        result += ASCII_LETTERS[distribution(generator)];
    }
    
    return result;
}

int main() {
    std::cout << "Test Case 1 (length=10): " << generateRandomString(10) << std::endl;
    std::cout << "Test Case 2 (length=5): " << generateRandomString(5) << std::endl;
    std::cout << "Test Case 3 (length=20): " << generateRandomString(20) << std::endl;
    std::cout << "Test Case 4 (length=1): " << generateRandomString(1) << std::endl;
    std::cout << "Test Case 5 (length=0): '" << generateRandomString(0) << "'" << std::endl;
    
    return 0;
}
