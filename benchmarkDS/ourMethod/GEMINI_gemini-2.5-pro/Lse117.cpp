#include <iostream>
#include <string>
#include <vector>
#include <random>

// Function to convert a float to a string
std::string floatToString(float f) {
    return std::to_string(f);
}

// Struct to hold the three strings for clarity
struct FloatStrings {
    std::string str_a;
    std::string str_b;
    std::string str_c;
};

// Function to generate three random floats and convert them to strings
FloatStrings generateThreeRandomFloatStrings() {
    // Use std::random_device for a non-deterministic seed, which is often
    // a source of cryptographic randomness on modern platforms.
    std::random_device rd;
    // Seed the Mersenne Twister engine
    std::mt19937 gen(rd());
    // Define the distribution for floats
    std::uniform_real_distribution<float> distrib(0.0f, 1000.0f);

    float a = distrib(gen);
    float b = distrib(gen);
    float c = distrib(gen);

    FloatStrings result;
    result.str_a = floatToString(a);
    result.str_b = floatToString(b);
    result.str_c = floatToString(c);
    
    return result;
}

int main() {
    std::cout << "Running 5 test cases..." << std::endl;
    for (int i = 0; i < 5; ++i) {
        std::cout << "\n--- Test Case " << i + 1 << " ---" << std::endl;
        FloatStrings results = generateThreeRandomFloatStrings();
        // The prompt asks for these specific variable names
        std::string str_a = results.str_a;
        std::string str_b = results.str_b;
        std::string str_c = results.str_c;

        std::cout << "str_a: " << str_a << std::endl;
        std::cout << "str_b: " << str_b << std::endl;
        std::cout << "str_c: " << str_c << std::endl;
    }
    return 0;
}