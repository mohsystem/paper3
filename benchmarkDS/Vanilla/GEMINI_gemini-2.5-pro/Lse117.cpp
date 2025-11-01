#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <ctime>

/**
 * Generates three random floats and converts them to strings.
 * @param gen A reference to a random number generator.
 * @return A std::vector<std::string> containing the three strings.
 */
std::vector<std::string> generateRandomFloatStrings(std::mt19937& gen) {
    std::uniform_real_distribution<float> dis(0.0f, 100.0f);
    
    // Generate three random floats
    float a = dis(gen);
    float b = dis(gen);
    float c = dis(gen);

    // Convert floats to strings
    std::string str_a = std::to_string(a);
    std::string str_b = std::to_string(b);
    std::string str_c = std::to_string(c);

    return {str_a, str_b, str_c};
}

int main() {
    // Seed the random number generator once using a random device
    std::random_device rd;
    std::mt19937 gen(rd());

    for (int i = 0; i < 5; ++i) {
        std::cout << "Test Case " << (i + 1) << ":" << std::endl;
        std::vector<std::string> results = generateRandomFloatStrings(gen);
        std::string str_a = results[0];
        std::string str_b = results[1];
        std::string str_c = results[2];

        std::cout << "  str_a: " << str_a << std::endl;
        std::cout << "  str_b: " << str_b << std::endl;
        std::cout << "  str_c: " << str_c << std::endl;
        std::cout << std::endl;
    }

    return 0;
}