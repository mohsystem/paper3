
#include <iostream>
#include <string>
#include <random>
#include <sstream>

std::string floatToString(float value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

int main() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    
    // Test case 1
    float a1 = dis(gen);
    float b1 = dis(gen);
    float c1 = dis(gen);
    std::string str_a1 = floatToString(a1);
    std::string str_b1 = floatToString(b1);
    std::string str_c1 = floatToString(c1);
    std::cout << "Test 1: " << str_a1 << ", " << str_b1 << ", " << str_c1 << std::endl;
    
    // Test case 2
    float a2 = dis(gen);
    float b2 = dis(gen);
    float c2 = dis(gen);
    std::string str_a2 = floatToString(a2);
    std::string str_b2 = floatToString(b2);
    std::string str_c2 = floatToString(c2);
    std::cout << "Test 2: " << str_a2 << ", " << str_b2 << ", " << str_c2 << std::endl;
    
    // Test case 3
    float a3 = dis(gen);
    float b3 = dis(gen);
    float c3 = dis(gen);
    std::string str_a3 = floatToString(a3);
    std::string str_b3 = floatToString(b3);
    std::string str_c3 = floatToString(c3);
    std::cout << "Test 3: " << str_a3 << ", " << str_b3 << ", " << str_c3 << std::endl;
    
    // Test case 4
    float a4 = dis(gen);
    float b4 = dis(gen);
    float c4 = dis(gen);
    std::string str_a4 = floatToString(a4);
    std::string str_b4 = floatToString(b4);
    std::string str_c4 = floatToString(c4);
    std::cout << "Test 4: " << str_a4 << ", " << str_b4 << ", " << str_c4 << std::endl;
    
    // Test case 5
    float a5 = dis(gen);
    float b5 = dis(gen);
    float c5 = dis(gen);
    std::string str_a5 = floatToString(a5);
    std::string str_b5 = floatToString(b5);
    std::string str_c5 = floatToString(c5);
    std::cout << "Test 5: " << str_a5 << ", " << str_b5 << ", " << str_c5 << std::endl;
    
    return 0;
}
