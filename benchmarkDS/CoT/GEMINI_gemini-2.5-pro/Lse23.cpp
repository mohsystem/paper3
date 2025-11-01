#include <iostream>
#include <random>

/**
 * @brief Generates three random integers larger than 1000, calculates their sum,
 *        and prints the results in a specific format.
 * 
 * @param gen A reference to the random number generator engine.
 * @param distrib A reference to the uniform integer distribution.
 */
void generateAndPrintRandomSum(std::mt19937& gen, std::uniform_int_distribution<>& distrib) {
    // Generate three random integers using the provided generator and distribution.
    int a = distrib(gen);
    int b = distrib(gen);
    int c = distrib(gen);

    // Calculate the sum. An 'int' is sufficient as the max sum is ~30000.
    int sum = a + b + c;

    // Print the result to the standard output.
    std::cout << "a : " << a << " b : " << b << " c : " << c
              << " sum: " << sum << " sum2: " << sum << std::endl;
}

int main() {
    // Use std::random_device to obtain a seed for the random number engine.
    std::random_device rd;
    // Standard mersenne_twister_engine seeded with rd().
    std::mt19937 gen(rd());
    // Define the distribution for numbers > 1000 (range is inclusive).
    std::uniform_int_distribution<> distrib(1001, 9999);
    
    // Run 5 test cases as requested.
    std::cout << "Running C++ Test Cases:" << std::endl;
    for (int i = 0; i < 5; ++i) {
        generateAndPrintRandomSum(gen, distrib);
    }

    return 0;
}