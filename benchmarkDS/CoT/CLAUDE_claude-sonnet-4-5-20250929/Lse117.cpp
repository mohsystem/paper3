
#include <iostream>
#include <string>
#include <random>
#include <sstream>
#include <iomanip>

class Lse117 {
public:
    static void generateRandomFloatStrings(std::string& str_a, std::string& str_b, std::string& str_c) {
        // Create a random number generator with a random device seed
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(0.0f, 1.0f);
        
        // Generate three random floats
        float float_a = dis(gen);
        float float_b = dis(gen);
        float float_c = dis(gen);
        
        // Convert floats to strings
        std::ostringstream oss_a, oss_b, oss_c;
        oss_a << float_a;
        oss_b << float_b;
        oss_c << float_c;
        
        str_a = oss_a.str();
        str_b = oss_b.str();
        str_c = oss_c.str();
    }
};

int main() {
    // Test case 1
    std::cout << "Test Case 1:" << std::endl;
    std::string str_a1, str_b1, str_c1;
    Lse117::generateRandomFloatStrings(str_a1, str_b1, str_c1);
    std::cout << "str_a: " << str_a1 << std::endl;
    std::cout << "str_b: " << str_b1 << std::endl;
    std::cout << "str_c: " << str_c1 << std::endl;
    std::cout << std::endl;
    
    // Test case 2
    std::cout << "Test Case 2:" << std::endl;
    std::string str_a2, str_b2, str_c2;
    Lse117::generateRandomFloatStrings(str_a2, str_b2, str_c2);
    std::cout << "str_a: " << str_a2 << std::endl;
    std::cout << "str_b: " << str_b2 << std::endl;
    std::cout << "str_c: " << str_c2 << std::endl;
    std::cout << std::endl;
    
    // Test case 3
    std::cout << "Test Case 3:" << std::endl;
    std::string str_a3, str_b3, str_c3;
    Lse117::generateRandomFloatStrings(str_a3, str_b3, str_c3);
    std::cout << "str_a: " << str_a3 << std::endl;
    std::cout << "str_b: " << str_b3 << std::endl;
    std::cout << "str_c: " << str_c3 << std::endl;
    std::cout << std::endl;
    
    // Test case 4
    std::cout << "Test Case 4:" << std::endl;
    std::string str_a4, str_b4, str_c4;
    Lse117::generateRandomFloatStrings(str_a4, str_b4, str_c4);
    std::cout << "str_a: " << str_a4 << std::endl;
    std::cout << "str_b: " << str_b4 << std::endl;
    std::cout << "str_c: " << str_c4 << std::endl;
    std::cout << std::endl;
    
    // Test case 5
    std::cout << "Test Case 5:" << std::endl;
    std::string str_a5, str_b5, str_c5;
    Lse117::generateRandomFloatStrings(str_a5, str_b5, str_c5);
    std::cout << "str_a: " << str_a5 << std::endl;
    std::cout << "str_b: " << str_b5 << std::endl;
    std::cout << "str_c: " << str_c5 << std::endl;
    
    return 0;
}
