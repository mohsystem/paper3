
#include <iostream>
#include <random>
#include <array>
#include <cstdlib>

// Use std::random_device for non-deterministic seed
// Use std::mt19937 for quality random generation
class SecureRandomGenerator {
private:
    std::random_device rd;
    std::mt19937 gen;
    
public:
    SecureRandomGenerator() : gen(rd()) {}
    
    int getRandomInRange(int min, int max) {
        // Validate range to prevent integer overflow
        if (min > max) {
            std::cerr << "Invalid range" << std::endl;
            return min;
        }
        std::uniform_int_distribution<int> dis(min, max);
        return dis(gen);
    }
};

std::array<int, 3> generateAndCalculateDifference(SecureRandomGenerator& rng) {
    // Generate random integers in range [10, 1000]
    int num1 = rng.getRandomInRange(10, 1000);
    int num2 = rng.getRandomInRange(10, 1000);
    
    // Calculate absolute difference using std::abs
    int difference = std::abs(num1 - num2);
    
    return {num1, num2, difference};
}

void printResults(int num1, int num2, int difference) {
    // Validate inputs are within expected range
    if (num1 < 10 || num1 > 1000 || num2 < 10 || num2 > 1000) {
        std::cerr << "Invalid input values" << std::endl;
        return;
    }
    
    // Use const format strings to prevent injection
    std::cout << "First number: " << num1 << std::endl;
    std::cout << "Second number: " << num2 << std::endl;
    std::cout << "Difference: " << difference << std::endl;
}

int main() {
    SecureRandomGenerator rng;
    
    // Test case 1
    std::cout << "Test Case 1:" << std::endl;
    auto result1 = generateAndCalculateDifference(rng);
    printResults(result1[0], result1[1], result1[2]);
    std::cout << std::endl;
    
    // Test case 2
    std::cout << "Test Case 2:" << std::endl;
    auto result2 = generateAndCalculateDifference(rng);
    printResults(result2[0], result2[1], result2[2]);
    std::cout << std::endl;
    
    // Test case 3
    std::cout << "Test Case 3:" << std::endl;
    auto result3 = generateAndCalculateDifference(rng);
    printResults(result3[0], result3[1], result3[2]);
    std::cout << std::endl;
    
    // Test case 4
    std::cout << "Test Case 4:" << std::endl;
    auto result4 = generateAndCalculateDifference(rng);
    printResults(result4[0], result4[1], result4[2]);
    std::cout << std::endl;
    
    // Test case 5
    std::cout << "Test Case 5:" << std::endl;
    auto result5 = generateAndCalculateDifference(rng);
    printResults(result5[0], result5[1], result5[2]);
    
    return 0;
}
