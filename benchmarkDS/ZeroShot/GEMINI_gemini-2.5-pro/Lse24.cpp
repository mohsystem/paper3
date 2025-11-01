#include <iostream>
#include <random>
#include <cmath>       // For std::abs
#include <stdexcept>   // For std::invalid_argument
#include <string>      // For std::to_string

// A struct to hold the results for cleaner return from the function.
struct RandomResult {
    int num1;
    int num2;
    int difference;
};

/**
 * Generates two random integers within a specified range using a provided
 * random number generator, calculates their absolute difference, and returns the results.
 * Using std::random_device to seed std::mt19937 is preferred for better random numbers.
 *
 * @param min The minimum value of the range (inclusive).
 * @param max The maximum value of the range (inclusive).
 * @param gen A reference to a seeded Mersenne Twister engine.
 * @return A RandomResult struct containing the two numbers and their difference.
 * @throws std::invalid_argument if min is greater than max.
 */
RandomResult generateRandomsAndDifference(int min, int max, std::mt19937& gen) {
    if (min > max) {
        throw std::invalid_argument("Min value cannot be greater than Max value.");
    }
    
    // This distribution ensures the numbers are uniform within the range [min, max].
    std::uniform_int_distribution<> distrib(min, max);
    
    int num1 = distrib(gen);
    int num2 = distrib(gen);
    int difference = std::abs(num1 - num2);
    
    return {num1, num2, difference};
}

int main() {
    // For security, seed the random number generator with a non-deterministic source.
    // std::random_device is a source of non-deterministic random numbers, if available.
    try {
        std::random_device rd;
        std::mt19937 gen(rd());

        std::cout << "Running 5 test cases..." << std::endl;
        const int MIN_VAL = 10;
        const int MAX_VAL = 1000;

        for (int i = 0; i < 5; ++i) {
            RandomResult result = generateRandomsAndDifference(MIN_VAL, MAX_VAL, gen);
            std::cout << "Test Case " << i + 1 << ": "
                      << "Num1: " << result.num1
                      << ", Num2: " << result.num2
                      << ", Difference: " << result.difference << std::endl;
        }
    } catch (const std::exception& e) {
        // std::random_device can throw if no non-deterministic source is available.
        std::cerr << "A fatal error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}