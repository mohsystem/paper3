
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include <ctime>

class Task128 {
public:
    // Generate random integer between min and max (inclusive)
    static int generateRandomInt(int min, int max) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(min, max);
        return dis(gen);
    }
    
    // Generate random double between 0.0 and 1.0
    static double generateRandomDouble() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 1.0);
        return dis(gen);
    }
    
    // Generate random alphanumeric token of specified length
    static std::string generateRandomToken(int length) {
        const std::string characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, characters.length() - 1);
        
        std::string token;
        for (int i = 0; i < length; i++) {
            token += characters[dis(gen)];
        }
        
        return token;
    }
    
    // Generate random boolean
    static bool generateRandomBoolean() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 1);
        return dis(gen) == 1;
    }
    
    // Generate vector of random integers
    static std::vector<int> generateRandomIntArray(int size, int min, int max) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(min, max);
        
        std::vector<int> array;
        for (int i = 0; i < size; i++) {
            array.push_back(dis(gen));
        }
        
        return array;
    }
};

int main() {
    std::cout << "Test Case 1: Random integer between 1 and 100" << std::endl;
    std::cout << Task128::generateRandomInt(1, 100) << std::endl;
    
    std::cout << "\\nTest Case 2: Random double" << std::endl;
    std::cout << Task128::generateRandomDouble() << std::endl;
    
    std::cout << "\\nTest Case 3: Random token of length 16" << std::endl;
    std::cout << Task128::generateRandomToken(16) << std::endl;
    
    std::cout << "\\nTest Case 4: Random boolean" << std::endl;
    std::cout << (Task128::generateRandomBoolean() ? "true" : "false") << std::endl;
    
    std::cout << "\\nTest Case 5: Array of 5 random integers between 10 and 50" << std::endl;
    std::vector<int> randomArray = Task128::generateRandomIntArray(5, 10, 50);
    for (int num : randomArray) {
        std::cout << num << " ";
    }
    std::cout << std::endl;
    
    return 0;
}
