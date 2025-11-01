#include <iostream>
#include <string>
#include <vector>
#include <random>

/**
 * @brief Generates three random floats using a secure random seed and converts them to strings.
 * 
 * @return A vector of three strings, each representing a random float.
 */
std::vector<std::string> generateAndConvertRandomFloats() {
    // Use std::random_device to obtain a non-deterministic random seed from the hardware.
    // This is the recommended practice for seeding pseudo-random number generators securely.
    std::random_device rd;
    
    // Seed the Mersenne Twister engine, a high-quality pseudo-random number generator.
    std::mt19937 gen(rd());
    
    // Define the distribution for floats between 0.0 and 1.0.
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);

    // Generate three random floats.
    float a = dis(gen);
    float b = dis(gen);
    float c = dis(gen);

    // Convert floats to strings using std::to_string and store in a vector.
    std::vector<std::string> result;
    result.push_back(std::to_string(a));
    result.push_back(std::to_string(b));
    result.push_back(std::to_string(c));

    return result;
}

int main() {
    // Run 5 test cases to demonstrate the functionality.
    for (int i = 0; i < 5; ++i) {
        std::cout << "Test Case " << i + 1 << ":" << std::endl;
        std::vector<std::string> randomStrings = generateAndConvertRandomFloats();
        
        // The resulting strings are stored in these variables as per the prompt.
        std::string str_a = randomStrings[0];
        std::string str_b = randomStrings[1];
        std::string str_c = randomStrings[2];

        std::cout << "  str_a: " << str_a << std::endl;
        std::cout << "  str_b: " << str_b << std::endl;
        std::cout << "  str_c: " << str_c << std::endl;
        std::cout << std::endl;
    }
    return 0;
}