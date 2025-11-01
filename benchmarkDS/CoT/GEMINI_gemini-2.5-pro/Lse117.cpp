#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <ctime>

/**
 * Generates three random floats and converts them to strings.
 * @return A vector of strings containing the three converted random floats.
 */
std::vector<std::string> generateAndConvertRandomFloats() {
    // Modern C++ random number generation, seeded once
    static std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<float> distrib(0.0f, 100.0f);
    
    // Generate three random floats
    float a = distrib(gen);
    float b = distrib(gen);
    float c = distrib(gen);
    
    // Convert floats to strings (safe conversion)
    std::string str_a = std::to_string(a);
    std::string str_b = std::to_string(b);
    std::string str_c = std::to_string(c);
    
    return {str_a, str_b, str_c};
}

int main() {
    std::cout << "Running 5 test cases..." << std::endl;
    for (int i = 1; i <= 5; ++i) {
        std::cout << "\n--- Test Case " << i << " ---" << std::endl;
        std::vector<std::string> results = generateAndConvertRandomFloats();
        std::string str_a = results[0];
        std::string str_b = results[1];
        std::string str_c = results[2];
        
        std::cout << "str_a: " << str_a << std::endl;
        std::cout << "str_b: " << str_b << std::endl;
        std::cout << "str_c: " << str_c << std::endl;
    }
    return 0;
}