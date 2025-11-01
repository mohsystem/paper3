#include <iostream>
#include <random>

/**
 * @brief Generates three random integers larger than 1000, calculates their sum,
 * and prints the results.
 */
void generateAndPrintRandomSum() {
    // Use std::random_device to seed the random number generator.
    // This provides a source of non-deterministic random numbers (if available).
    std::random_device rd;
    // Use the Mersenne Twister engine, a high-quality pseudo-random number generator.
    std::mt19937 gen(rd());
    // Define the distribution for integers in the range [1001, 9999].
    std::uniform_int_distribution<> distrib(1001, 9999);

    // Generate three random integers larger than 1000.
    int a = distrib(gen);
    int b = distrib(gen);
    int c = distrib(gen);

    // Add the three integers.
    // Using long long is a good practice for sums to avoid overflow,
    // although an int would suffice for the chosen range.
    long long sum = static_cast<long long>(a) + b + c;

    // Print the results in the specified format.
    std::cout << "a : " << a << " b : " << b << " c : " << c
              << " sum: " << sum << " sum2: " << sum << std::endl;
}

/**
 * @brief Main function to run 5 test cases.
 * @return int Exit code.
 */
int main() {
    // Run 5 test cases as requested.
    for (int i = 0; i < 5; ++i) {
        generateAndPrintRandomSum();
    }
    return 0;
}