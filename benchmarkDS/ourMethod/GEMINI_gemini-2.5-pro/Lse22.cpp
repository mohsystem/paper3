#include <iostream>
#include <random>
#include <limits>

/**
 * @brief Generates a random integer greater than 1000.
 *
 * This function uses the C++11 <random> library for high-quality random numbers.
 * It is seeded with a hardware entropy source if available, making it suitable
 * for many applications.
 *
 * @return A random integer greater than 1000.
 */
int generateRandomNumber() {
    // Static ensures these variables are initialized only once, providing a better
    // random distribution over multiple calls and improving performance.
    
    // Use std::random_device to obtain a non-deterministic seed from the OS/hardware.
    static std::random_device rd;
    
    // Seed the Mersenne Twister engine, a high-quality pseudo-random number generator.
    static std::mt19937 gen(rd());
    
    // Define the distribution. The number must be greater than 1000,
    // so the range starts at 1001. The upper bound is the maximum value for an int.
    static std::uniform_int_distribution<int> distrib(1001, std::numeric_limits<int>::max());
    
    return distrib(gen);
}

int main() {
    std::cout << "Generating 5 random numbers greater than 1000:" << std::endl;
    for (int i = 0; i < 5; ++i) {
        std::cout << "Test Case " << i + 1 << ": " << generateRandomNumber() << std::endl;
    }
    return 0;
}