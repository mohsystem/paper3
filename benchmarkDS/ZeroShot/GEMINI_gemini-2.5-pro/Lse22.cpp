#include <iostream>
#include <random>
#include <limits>

/**
 * @brief Generates a cryptographically secure random number greater than 1000.
 * 
 * This function uses std::random_device, which is the standard C++ interface
 * to a non-deterministic random number generator (often hardware-based).
 * It's intended for security-sensitive contexts.
 * 
 * @return A random integer greater than 1000. Returns -1 on failure to
 *         obtain a random number.
 */
int generateRandomNumberGreaterThan1000() {
    try {
        // Use std::random_device as a source of non-deterministic random numbers.
        std::random_device rd;
        
        // Define the uniform distribution for the desired range.
        // The range is [1001, max_int].
        std::uniform_int_distribution<int> distrib(1001, std::numeric_limits<int>::max());
        
        // Generate and return the random number.
        return distrib(rd);
    } catch (const std::exception& e) {
        // std::random_device may throw an exception if a non-deterministic source is not available.
        std::cerr << "Error generating random number: " << e.what() << std::endl;
        return -1;
    }
}

int main() {
    std::cout << "Generating 5 random numbers greater than 1000:" << std::endl;
    for (int i = 0; i < 5; ++i) {
        int random_num = generateRandomNumberGreaterThan1000();
        if (random_num != -1) {
            std::cout << "Test Case " << (i + 1) << ": " << random_num << std::endl;
        }
    }
    return 0;
}