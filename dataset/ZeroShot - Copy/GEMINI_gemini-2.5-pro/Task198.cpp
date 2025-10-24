#include <iostream>
#include <vector>
#include <random>

// The given API, implemented for testing purposes using C++11 random library.
// This ensures a high-quality uniform distribution for rand7.
int rand7() {
    // static ensures the generator is seeded only once.
    static std::mt19937 generator(std::random_device{}());
    static std::uniform_int_distribution<int> distribution(1, 7);
    return distribution(generator);
}

/**
 * @brief Generates a uniform random integer in the range [1, 10] using only rand7().
 * 
 * This implementation uses rejection sampling to ensure a uniform distribution.
 * 1. Two calls to rand7() can generate a uniform random number from 1 to 49.
 *    idx = (rand7() - 1) * 7 + rand7()
 * 2. We take a range that is a multiple of 10, in this case, 1 to 40.
 * 3. If the generated number `idx` is outside this range (41-49), we reject it
 *    and try again. This is crucial for uniformity.
 * 4. If the number is within the range [1, 40], we map it to [1, 10] using
 *    the modulo operator.
 * 
 * @return int A random integer between 1 and 10, inclusive.
 */
int rand10() {
    int result;
    while (true) {
        // This generates a uniformly random number in the range [1, 49].
        result = (rand7() - 1) * 7 + rand7();
        
        // We only accept results in the range [1, 40] to ensure uniformity.
        if (result <= 40) {
            // Map the number from [1, 40] to [1, 10].
            return (result - 1) % 10 + 1;
        }
        // If the result is > 40 (i.e., 41-49), we reject it and loop again.
    }
}

int main() {
    int testCases[] = {1, 2, 3, 10, 50};
    for (int n : testCases) {
        std::cout << "Test Case (n=" << n << "):" << std::endl;
        std::vector<int> results;
        for (int i = 0; i < n; ++i) {
            results.push_back(rand10());
        }

        std::cout << "[";
        for (size_t i = 0; i < results.size(); ++i) {
            std::cout << results[i] << (i == results.size() - 1 ? "" : ", ");
        }
        std::cout << "]" << std::endl;
    }
    return 0;
}