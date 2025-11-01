
#include <iostream>
#include <string>
#include <array>
#include <random>
#include <cstdio>
#include <stdexcept>

/**
 * Generates three random floats and converts them to strings.
 * Returns an array containing the three string representations.
 * 
 * Security measures:
 * - Uses std::random_device and std::mt19937 for better randomness than rand()
 * - Uses snprintf with explicit buffer size to prevent buffer overflow
 * - Validates buffer size and snprintf return value to prevent truncation
 * - Uses std::array for automatic memory management
 * - Checks for format errors and truncation
 */
std::array<std::string, 3> generateRandomFloatStrings() {
    // Use std::random_device for seeding and std::mt19937 for generation
    // This provides better randomness than std::rand()
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    
    // Generate three random floats
    float random1 = dis(gen);
    float random2 = dis(gen);
    float random3 = dis(gen);
    
    // Define buffer size with sufficient space for float string representation
    // Maximum float string length is approximately 47 characters including null terminator
    const size_t BUFFER_SIZE = 64;
    char buffer1[BUFFER_SIZE] = {0};  // Initialize to zero
    char buffer2[BUFFER_SIZE] = {0};
    char buffer3[BUFFER_SIZE] = {0};
    
    // Convert to strings using snprintf (safe sprintf alternative)
    // snprintf guarantees null termination and returns number of characters that would be written
    int ret1 = snprintf(buffer1, BUFFER_SIZE, "%f", random1);
    int ret2 = snprintf(buffer2, BUFFER_SIZE, "%f", random2);
    int ret3 = snprintf(buffer3, BUFFER_SIZE, "%f", random3);
    
    // Validate snprintf results - check for errors and truncation
    if (ret1 < 0 || ret1 >= static_cast<int>(BUFFER_SIZE) ||
        ret2 < 0 || ret2 >= static_cast<int>(BUFFER_SIZE) ||
        ret3 < 0 || ret3 >= static_cast<int>(BUFFER_SIZE)) {
        throw std::runtime_error("String conversion failed or truncated");
    }
    
    // Create strings from buffers - buffers are guaranteed null-terminated by snprintf
    std::string string1(buffer1);
    std::string string2(buffer2);
    std::string string3(buffer3);
    
    return {string1, string2, string3};
}

int main() {
    try {
        // Test case 1
        std::cout << "Test case 1:" << std::endl;
        auto result1 = generateRandomFloatStrings();
        std::cout << "Random float 1: " << result1[0] << std::endl;
        std::cout << "Random float 2: " << result1[1] << std::endl;
        std::cout << "Random float 3: " << result1[2] << std::endl;
        std::cout << std::endl;
        
        // Test case 2
        std::cout << "Test case 2:" << std::endl;
        auto result2 = generateRandomFloatStrings();
        std::cout << "Random float 1: " << result2[0] << std::endl;
        std::cout << "Random float 2: " << result2[1] << std::endl;
        std::cout << "Random float 3: " << result2[2] << std::endl;
        std::cout << std::endl;
        
        // Test case 3
        std::cout << "Test case 3:" << std::endl;
        auto result3 = generateRandomFloatStrings();
        std::cout << "Random float 1: " << result3[0] << std::endl;
        std::cout << "Random float 2: " << result3[1] << std::endl;
        std::cout << "Random float 3: " << result3[2] << std::endl;
        std::cout << std::endl;
        
        // Test case 4
        std::cout << "Test case 4:" << std::endl;
        auto result4 = generateRandomFloatStrings();
        std::cout << "Random float 1: " << result4[0] << std::endl;
        std::cout << "Random float 2: " << result4[1] << std::endl;
        std::cout << "Random float 3: " << result4[2] << std::endl;
        std::cout << std::endl;
        
        // Test case 5
        std::cout << "Test case 5:" << std::endl;
        auto result5 = generateRandomFloatStrings();
        std::cout << "Random float 1: " << result5[0] << std::endl;
        std::cout << "Random float 2: " << result5[1] << std::endl;
        std::cout << "Random float 3: " << result5[2] << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
