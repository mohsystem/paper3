#include <iostream>
#include <vector>
#include <random>
#include <chrono>

// The rand7() API is provided.
// This is a mock implementation for testing purposes.
std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
int rand7() {
    std::uniform_int_distribution<int> dist(1, 7);
    return dist(rng);
}

/**
 * @brief Generates a uniform random integer in the range [1, 10]
 * using only the rand7() API.
 * @return A random integer from 1 to 10.
 */
int rand10() {
    int result;
    do {
        // Generate a uniform random number in [1, 49] by calling rand7() twice.
        // This is equivalent to generating a 2-digit number in base 7.
        result = (rand7() - 1) * 7 + rand7();
    } while (result > 40); // Use numbers in [1, 40] and reject others.
                           // This is rejection sampling.
    
    // Map the number from [1, 40] to [1, 10]
    return (result - 1) % 10 + 1;
}

void run_test_case(int n) {
    std::cout << "n = " << n << ": [";
    if (n > 0) {
        std::cout << rand10();
    }
    for (int i = 1; i < n; ++i) {
        std::cout << ", " << rand10();
    }
    std::cout << "]" << std::endl;
}

int main() {
    std::cout << "Running 5 test cases..." << std::endl;
    run_test_case(1);
    run_test_case(2);
    run_test_case(3);
    run_test_case(10);
    run_test_case(20);
    return 0;
}