
#include <iostream>
#include <string>
#include <random>

class Task85 {
private:
    static const std::string ASCII_LETTERS;
    static std::random_device rd;
    static std::mt19937 gen;
    
public:
    static std::string generateRandomString(int length) {
        if (length < 0) {
            throw std::invalid_argument("Length must be non-negative");
        }
        
        std::uniform_int_distribution<> dis(0, ASCII_LETTERS.length() - 1);
        std::string result;
        result.reserve(length);
        
        for (int i = 0; i < length; i++) {
            result += ASCII_LETTERS[dis(gen)];
        }
        
        return result;
    }
};

const std::string Task85::ASCII_LETTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
std::random_device Task85::rd;
std::mt19937 Task85::gen(Task85::rd());

int main() {
    std::cout << "Test case 1 (length 10): " << Task85::generateRandomString(10) << std::endl;
    std::cout << "Test case 2 (length 5): " << Task85::generateRandomString(5) << std::endl;
    std::cout << "Test case 3 (length 20): " << Task85::generateRandomString(20) << std::endl;
    std::cout << "Test case 4 (length 0): " << Task85::generateRandomString(0) << std::endl;
    std::cout << "Test case 5 (length 15): " << Task85::generateRandomString(15) << std::endl;
    
    return 0;
}
