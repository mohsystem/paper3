#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <stdexcept>

/**
 * @brief Generates three random floats using a generator seeded with a
 *        cryptographically secure source and returns them as strings.
 *
 * @return A std::vector<std::string> containing three strings, each representing a random float.
 */
std::vector<std::string> generateRandomFloatStrings() {
    // std::random_device provides non-deterministic random numbers if available,
    // often from a hardware entropy source or an OS-level CSPRNG.
    std::random_device rd;
    
    // Seed the Mersenne Twister engine.
    std::mt19937 gen(rd());
    
    // Define a distribution for floats between 0.0 and 1.0.
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);

    std::vector<std::string> result;
    result.reserve(3);
    for (int i = 0; i < 3; ++i) {
        float randomFloat = dis(gen);
        result.push_back(std::to_string(randomFloat));
    }
    return result;
}

int main() {
    std::cout << "Running 5 test cases..." << std::endl;
    for (int i = 0; i < 5; ++i) {
        std::cout << "Test Case " << i + 1 << ":" << std::endl;
        try {
            std::vector<std::string> randomStrings = generateRandomFloatStrings();
            if (randomStrings.size() == 3) {
                std::cout << "  String 1: " << randomStrings[0] << std::endl;
                std::cout << "  String 2: " << randomStrings[1] << std::endl;
                std::cout << "  String 3: " << randomStrings[2] << std::endl;
            } else {
                std::cerr << "  Error: Did not generate 3 strings." << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "  An exception occurred: " << e.what() << std::endl;
        }
        std::cout << std::endl;
    }
    return 0;
}