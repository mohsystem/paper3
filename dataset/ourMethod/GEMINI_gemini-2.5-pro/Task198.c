#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// The rand7() API is provided.
// This is a mock implementation for testing purposes.
int rand7() {
    return (rand() % 7) + 1;
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
    printf("n = %d: [", n);
    if (n > 0) {
        printf("%d", rand10());
    }
    for (int i = 1; i < n; ++i) {
        printf(", %d", rand10());
    }
    printf("]\n");
}

int main() {
    // Seed the random number generator once.
    srand(time(NULL));

    printf("Running 5 test cases...\n");
    run_test_case(1);
    run_test_case(2);
    run_test_case(3);
    run_test_case(10);
    run_test_case(20);
    
    return 0;
}