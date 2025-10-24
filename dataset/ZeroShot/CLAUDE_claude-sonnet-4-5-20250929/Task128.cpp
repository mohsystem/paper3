
#include <iostream>
#include <string>
#include <random>
#include <sstream>
#include <iomanip>
#include <stdexcept>

class Task128 {
private:
    static std::random_device rd;
    static std::mt19937_64 generator;
    
public:
    // Generate random integer within a range
    static int generateRandomInt(int min, int max) {
        if (min >= max) {
            throw std::invalid_argument("Max must be greater than min");
        }
        std::uniform_int_distribution<int> distribution(min, max - 1);
        return distribution(generator);
    }
    
    // Generate random long
    static long long generateRandomLong() {
        std::uniform_int_distribution<long long> distribution;
        return distribution(generator);
    }
    
    // Generate random token (hex encoded)
    static std::string generateRandomToken(int byteLength) {
        if (byteLength <= 0) {
            throw std::invalid_argument("Byte length must be positive");
        }
        std::uniform_int_distribution<int> distribution(0, 255);
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (int i = 0; i < byteLength; i++) {
            ss << std::setw(2) << distribution(generator);
        }
        return ss.str();
    }
    
    // Generate random alphanumeric string
    static std::string generateRandomAlphanumeric(int length) {
        if (length <= 0) {
            throw std::invalid_argument("Length must be positive");
        }
        const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
        std::uniform_int_distribution<size_t> distribution(0, chars.length() - 1);
        std::string result;
        result.reserve(length);
        for (int i = 0; i < length; i++) {
            result += chars[distribution(generator)];
        }
        return result;
    }
};

std::random_device Task128::rd;
std::mt19937_64 Task128::generator(Task128::rd());

int main() {
    std::cout << "Test Case 1 - Random Integer (1-100):" << std::endl;
    std::cout << Task128::generateRandomInt(1, 100) << std::endl;
    
    std::cout << "\\nTest Case 2 - Random Long:" << std::endl;
    std::cout << Task128::generateRandomLong() << std::endl;
    
    std::cout << "\\nTest Case 3 - Random Token (32 bytes):" << std::endl;
    std::cout << Task128::generateRandomToken(32) << std::endl;
    
    std::cout << "\\nTest Case 4 - Random Alphanumeric (16 chars):" << std::endl;
    std::cout << Task128::generateRandomAlphanumeric(16) << std::endl;
    
    std::cout << "\\nTest Case 5 - Multiple Random Integers:" << std::endl;
    for (int i = 0; i < 5; i++) {
        std::cout << Task128::generateRandomInt(1, 1000) << std::endl;
    }
    
    return 0;
}
