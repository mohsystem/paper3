#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// The given API, implemented for testing purposes.
// Using rand() from stdlib.h. It should be seeded once in main.
int rand7() {
    return rand() % 7 + 1;
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
    while (1) { // Infinite loop
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
    // Seed the random number generator once at the start of the program.
    srand(time(NULL));

    int testCases[] = {1, 2, 3, 10, 50};
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < numTestCases; ++i) {
        int n = testCases[i];
        printf("Test Case (n=%d):\n", n);
        printf("[");
        for (int j = 0; j < n; ++j) {
            printf("%d", rand10());
            if (j < n - 1) {
                printf(", ");
            }
        }
        printf("]\n");
    }

    return 0;
}