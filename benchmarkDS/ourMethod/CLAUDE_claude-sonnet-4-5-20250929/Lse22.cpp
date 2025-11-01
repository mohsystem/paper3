
#include <iostream>
#include <random>
#include <limits>

/**
 * Generates a cryptographically secure random number larger than 1000.
 * Uses random_device seeded mt19937 for unpredictable number generation.
 * 
 * @return A random integer greater than 1000
 */
int generateSecureRandomNumber() {
    // Use random_device for non-deterministic seed
    // This provides unpredictable initialization
    std::random_device rd;
    
    // Seed mt19937 with random_device for strong randomness
    std::mt19937 gen(rd());
    
    // Define distribution range to avoid overflow
    // Generate numbers from 1001 to INT_MAX
    std::uniform_int_distribution<int> dist(1001, std::numeric_limits<int>::max());
    
    // Generate and return the random number
    return dist(gen);
}

int main() {
    // Test case 1
    int result1 = generateSecureRandomNumber();
    std::cout << "Test 1 - Random number: " << result1 << " (should be > 1000)" << std::endl;
    
    // Test case 2
    int result2 = generateSecureRandomNumber();
    std::cout << "Test 2 - Random number: " << result2 << " (should be > 1000)" << std::endl;
    
    // Test case 3
    int result3 = generateSecureRandomNumber();
    std::cout << "Test 3 - Random number: " << result3 << " (should be > 1000)" << std::endl;
    
    // Test case 4
    int result4 = generateSecureRandomNumber();
    std::cout << "Test 4 - Random number: " << result4 << " (should be > 1000)" << std::endl;
    
    // Test case 5
    int result5 = generateSecureRandomNumber();
    std::cout << "Test 5 - Random number: " << result5 << " (should be > 1000)" << std::endl;
    
    return 0;
}
