#include <iostream>
#include <random>
#include <chrono>

/**
 * @brief Generates two random integers between 10 and 1000, calculates their
 * difference, and prints the results.
 * 
 * Uses a static random number generator to ensure it is seeded only once
 * for better performance and randomness across multiple calls.
 */
void generateAndCalculateDifference() {
    // Use a modern C++ random number generation approach.
    // The engine is seeded once using a high-resolution clock.
    static std::mt19937 gen(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<> distrib(10, 1000);

    // Generate the first random integer between 10 and 1000 (inclusive).
    int num1 = distrib(gen);

    // Generate the second random integer between 10 and 1000 (inclusive).
    int num2 = distrib(gen);

    // Calculate the difference between the two integers.
    int difference = num1 - num2;

    // Print the results.
    std::cout << "First number: " << num1 << std::endl;
    std::cout << "Second number: " << num2 << std::endl;
    std::cout << "Difference (First - Second): " << difference << std::endl;
}

int main() {
    std::cout << "Running 5 test cases:" << std::endl;
    for (int i = 0; i < 5; ++i) {
        std::cout << "--- Test Case " << i + 1 << " ---" << std::endl;
        generateAndCalculateDifference();
    }
    return 0;
}