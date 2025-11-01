#include <iostream>
#include <string>
#include <vector>
#include <random>   // For modern, high-quality random number generation
#include <sstream>  // For safe string conversion, an alternative to sprintf

/**
 * Generates three random floats and converts them to strings using a secure method.
 *
 * This implementation uses C++11 <random> for better random number generation
 * than C-style rand(). It uses std::stringstream for type-safe and
 * buffer-overflow-safe string conversion, which is the C++ equivalent to
 * the functionality of C's sprintf.
 *
 * @return A std::vector<std::string> containing three strings.
 */
std::vector<std::string> generateRandomFloatStrings() {
    // Use a modern C++ random number engine. It provides higher quality
    // randomness than std::rand().
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distrib(0.0f, 1.0f);

    float f1 = distrib(gen);
    float f2 = distrib(gen);
    float f3 = distrib(gen);

    std::vector<std::string> result;
    std::stringstream ss;

    // Convert floats to strings safely using stringstream
    ss << f1;
    result.push_back(ss.str());
    ss.str(""); // Clear the stream

    ss << f2;
    result.push_back(ss.str());
    ss.str("");

    ss << f3;
    result.push_back(ss.str());

    return result;
}

int main() {
    // 5 test cases
    for (int i = 0; i < 5; ++i) {
        std::cout << "Test Case " << (i + 1) << ":" << std::endl;
        std::vector<std::string> randomStrings = generateRandomFloatStrings();
        if (randomStrings.size() == 3) {
            std::cout << "string: " << randomStrings[0] << std::endl;
            std::cout << "string2: " << randomStrings[1] << std::endl;
            std::cout << "string3: " << randomStrings[2] << std::endl;
        }
        std::cout << std::endl;
    }
    return 0;
}