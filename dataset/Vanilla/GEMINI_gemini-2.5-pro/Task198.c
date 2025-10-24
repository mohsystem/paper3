#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// The given API rand7(), simulated for testing purposes.
int rand7() {
    return rand() % 7 + 1;
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
    while (1) {
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
    printf("[");
    for (int i = 0; i < n; ++i) {
        printf("%d", rand10());
        if (i < n - 1) {
            printf(",");
        }
    }
    printf("]\n");
}

int main() {
    // Seed the random number generator once at the beginning of the program.
    srand(time(NULL));

    // Test cases
    run_test_case(1);
    run_test_case(2);
    run_test_case(3);
    run_test_case(5);
    run_test_case(10);

    return 0;
}