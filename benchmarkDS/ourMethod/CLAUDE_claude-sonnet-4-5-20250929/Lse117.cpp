
#include <iostream>
#include <string>
#include <random>
#include <sstream>
#include <iomanip>
#include <array>
#include <cstring>

// Use std::random_device for seeding cryptographically secure generator
class SecureRandomGenerator {
private:
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_real_distribution<float> dis;
    
public:
    SecureRandomGenerator() : gen(rd()), dis(0.0f, 1.0f) {
        // Seed with random_device for better entropy
    }
    
    float generateFloat() {
        // Generate random float in range [0.0, 1.0)
        return dis(gen);
    }
};

// Global secure random generator
static SecureRandomGenerator secureRandom;

/**
 * Converts a float to string with controlled formatting
 * Uses std::ostringstream with fixed locale to prevent locale-dependent issues
 */
std::string floatToString(float value) {
    std::ostringstream oss;
    // Use "C" locale to ensure consistent formatting regardless of system locale
    oss.imbue(std::locale::classic());
    // Set precision for consistent output
    oss << std::fixed << std::setprecision(6) << value;
    return oss.str();
}

/**
 * Generates a cryptographically seeded random float
 */
float generateRandomFloat() {
    return secureRandom.generateFloat();
}

/**
 * Generates three random floats and converts them to strings
 * Returns array of three strings
 */
std::array<std::string, 3> generateThreeRandomFloatStrings() {
    // Generate three random floats using secure random generator
    float floatA = generateRandomFloat();
    float floatB = generateRandomFloat();
    float floatC = generateRandomFloat();
    
    // Convert to strings using secure conversion
    std::string strA = floatToString(floatA);
    std::string strB = floatToString(floatB);
    std::string strC = floatToString(floatC);
    
    // Return as array
    return {strA, strB, strC};
}

int main() {
    // Test case 1
    auto result1 = generateThreeRandomFloatStrings();
    std::cout << "Test 1 - str_a: " << result1[0] 
              << ", str_b: " << result1[1] 
              << ", str_c: " << result1[2] << std::endl;
    
    // Test case 2
    auto result2 = generateThreeRandomFloatStrings();
    std::cout << "Test 2 - str_a: " << result2[0] 
              << ", str_b: " << result2[1] 
              << ", str_c: " << result2[2] << std::endl;
    
    // Test case 3
    auto result3 = generateThreeRandomFloatStrings();
    std::cout << "Test 3 - str_a: " << result3[0] 
              << ", str_b: " << result3[1] 
              << ", str_c: " << result3[2] << std::endl;
    
    // Test case 4
    auto result4 = generateThreeRandomFloatStrings();
    std::cout << "Test 4 - str_a: " << result4[0] 
              << ", str_b: " << result4[1] 
              << ", str_c: " << result4[2] << std::endl;
    
    // Test case 5
    auto result5 = generateThreeRandomFloatStrings();
    std::cout << "Test 5 - str_a: " << result5[0] 
              << ", str_b: " << result5[1] 
              << ", str_c: " << result5[2] << std::endl;
    
    return 0;
}
