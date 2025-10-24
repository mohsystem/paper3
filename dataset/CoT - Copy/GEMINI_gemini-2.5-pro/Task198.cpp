#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

// This is the mock implementation of the provided API for testing purposes.
// The actual solution for rand10() does not use this directly.
int rand7() {
    return (rand() % 7) + 1;
}

/**
 * @brief Given the API rand7() that generates a uniform random integer in the range [1, 7],
 * this function generates a uniform random integer in the range [1, 10].
 * 
 * @return int A random integer in the range [1, 10].
 */
int rand10() {
    int row, col, idx;
    do {
        row = rand7();
        col = rand7();
        idx = (row - 1) * 7 + col; // Generates a uniform number in [1, 49]
    } while (idx > 40); // Rejection sampling: reject values that would cause non-uniformity

    // Map the uniform range [1, 40] to [1, 10]
    return (idx - 1) % 10 + 1;
}

void run_test(int test_num, int n) {
    std::cout << "Test Case " << test_num << " (n=" << n << "): [";
    for (int i = 0; i < n; ++i) {
        std::cout << rand10() << (i == n - 1 ? "" : ", ");
    }
    std::cout << "]" << std::endl;
}

int main() {
    // Seed the random number generator once for the mock rand7()
    srand(time(0));

    // Test cases
    run_test(1, 1);
    run_test(2, 2);
    run_test(3, 3);
    run_test(4, 5);
    run_test(5, 10);
    
    return 0;
}