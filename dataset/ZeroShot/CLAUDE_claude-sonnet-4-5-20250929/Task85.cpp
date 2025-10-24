
#include <iostream>
#include <string>
#include <random>
#include <stdexcept>

class Task85 {
public:
    static std::string generateRandomString(int length) {
        if (length < 0) {
            throw std::invalid_argument("Length must be non-negative");
        }
        
        const std::string ASCII_LETTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        std::random_device rd;
        std::mt19937 generator(rd());
        std::uniform_int_distribution<> distribution(0, ASCII_LETTERS.length() - 1);
        
        std::string result;
        result.reserve(length);
        
        for (int i = 0; i < length; i++) {
            result += ASCII_LETTERS[distribution(generator)];
        }
        
        return result;
    }
};

int main() {
    // Test case 1: Generate string of length 10
    std::cout << "Test 1 (length 10): " << Task85::generateRandomString(10) << std::endl;
    
    // Test case 2: Generate string of length 5
    std::cout << "Test 2 (length 5): " << Task85::generateRandomString(5) << std::endl;
    
    // Test case 3: Generate string of length 20
    std::cout << "Test 3 (length 20): " << Task85::generateRandomString(20) << std::endl;
    
    // Test case 4: Generate string of length 0
    std::cout << "Test 4 (length 0): '" << Task85::generateRandomString(0) << "'" << std::endl;
    
    // Test case 5: Generate string of length 15
    std::cout << "Test 5 (length 15): " << Task85::generateRandomString(15) << std::endl;
    
    return 0;
}
