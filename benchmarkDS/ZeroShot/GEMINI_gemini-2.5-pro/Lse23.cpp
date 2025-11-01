#include <iostream>
#include <random>    // For std::random_device, std::mt19937, std::uniform_int_distribution
#include <cstdint>   // For int64_t to prevent overflow

/**
 * @brief Generates three random integers greater than 1000, prints them,
 * calculates their sum safely, and prints the sum.
 *
 * This function uses C++11's <random> library for high-quality random numbers,
 * which is a significant improvement over legacy C-style rand().
 * std::random_device is used to seed the Mersenne Twister engine (mt19937)
 * to provide non-deterministic random values where available.
 */
void generateAndPrintSum() {
    // Use std::random_device to seed the random number engine. This provides
    // non-deterministic random numbers if the hardware supports it.
    std::random_device rd;
    // The Mersenne Twister engine is a high-quality PRNG.
    std::mt19937 gen(rd());
    // Define the distribution for integers in the range [1001, 30000].
    std::uniform_int_distribution<> distrib(1001, 30000);

    int a = distrib(gen);
    int b = distrib(gen);
    int c = distrib(gen);

    // Use a 64-bit integer (int64_t or long long) for the sum to prevent
    // signed integer overflow, which is undefined behavior and a security risk in C++.
    // A standard 32-bit int can hold up to ~2.1 billion. The sum of three ints
    // could exceed this, but will not exceed the capacity of a 64-bit integer.
    int64_t sum = static_cast<int64_t>(a) + b + c;

    // Print the numbers and their sum in the specified format.
    std::cout << "a : " << a << " b : " << b << " c : " << c
              << " sum: " << sum << " sum2: " << sum << std::endl;
}

int main() {
    std::cout << "CPP execution:" << std::endl;
    // Run 5 test cases as requested.
    for (int i = 0; i < 5; ++i) {
        generateAndPrintSum();
    }
    return 0;
}