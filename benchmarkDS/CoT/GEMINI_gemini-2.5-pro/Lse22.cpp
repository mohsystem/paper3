#include <iostream>
#include <random>
#include <limits>

/**
 * @brief Generates a random integer greater than 1000.
 *
 * This function uses C++11's <random> library for high-quality pseudo-random
 * number generation. It is thread-safe on C++11 and later.
 *
 * @return A random integer in the range [1001, INT_MAX].
 */
int generateRandomNumber() {
    // static ensures the generator is seeded only once, improving performance
    // and preventing re-seeding with the same value on rapid successive calls.
    
    // 1. Use std::random_device to obtain a non-deterministic seed from the OS.
    static std::random_device rd;
    
    // 2. Seed the Mersenne Twister engine. It's a high-quality PRNG.
    static std::mt19937 gen(rd());
    
    // 3. Define the distribution for the random numbers. We want integers
    //    greater than 1000, so the range is [1001, max_int].
    static std::uniform_int_distribution<> distrib(1001, std::numeric_limits<int>::max());
    
    // 4. Generate and return a number from the distribution.
    return distrib(gen);
}

int main() {
    std::cout << "Generating 5 random numbers greater than 1000:" << std::endl;
    for (int i = 0; i < 5; ++i) {
        std::cout << "Test Case " << (i + 1) << ": " << generateRandomNumber() << std::endl;
    }
    return 0;
}