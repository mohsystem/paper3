#include <iostream>
#include <vector>
#include <random>
#include <ctime>

// The given API rand7(), simulated for testing purposes.
// Using a static generator to ensure it's seeded only once per program run.
int rand7() {
    static std::mt19937 gen(time(nullptr));
    std::uniform_int_distribution<> distrib(1, 7);
    return distrib(gen);
}

/**
 * Generates a uniform random integer in the range [1, 10] using rand7().
 * This implementation uses rejection sampling.
 * 1. Generate a number in a larger range: (rand7() - 1) * 7 + rand7() gives a
 *    uniform number in [1, 49].
 * 2. Accept numbers in the range [1, 40] to map to [1, 10].
 * 3. Reject numbers in the range [41, 49] and retry.
 */
int rand10() {
    int result;
    while (true) {
        // Generate a number from 1 to 49 uniformly
        result = (rand7() - 1) * 7 + rand7();

        // If the number is in the desired range [1, 40], we can use it
        if (result <= 40) {
            // Map the number from [1, 40] to [1, 10]
            return (result - 1) % 10 + 1;
        }
        // Otherwise, reject and try again
    }
}

void run_test_case(int n) {
    std::vector<int> result;
    for (int i = 0; i < n; ++i) {
        result.push_back(rand10());
    }
    std::cout << "[";
    for (size_t i = 0; i < result.size(); ++i) {
        std::cout << result[i] << (i == result.size() - 1 ? "" : ",");
    }
    std::cout << "]" << std::endl;
}

int main() {
    // Test cases
    run_test_case(1);
    run_test_case(2);
    run_test_case(3);
    run_test_case(5);
    run_test_case(10);
    
    return 0;
}