#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/**
 * @brief Generates a pseudo-random integer greater than 1000.
 *
 * NOTE: This function uses rand(), which is not cryptographically secure and
 * should not be used for security-sensitive applications. The random number 
 * generator must be seeded once before calling this function, for example,
 * by calling srand(time(NULL)) in the main function.
 *
 * @return A pseudo-random integer in the range [1001, 50000].
 */
int generateRandomNumber() {
    // The number must be > 1000, so the minimum value is 1001.
    int min = 1001;
    
    // We choose an arbitrary upper bound. Using rand() with very large ranges
    // can be problematic if RAND_MAX is small.
    int max = 50000;
    
    // Formula for a number in [min, max]: min + rand() % (max - min + 1)
    // This modulo operation can introduce a slight bias if (max - min + 1)
    // does not evenly divide RAND_MAX + 1. For non-critical applications,
    // this is often considered acceptable.
    return min + rand() % (max - min + 1);
}

int main(void) {
    // Seed the random number generator once at the start of the program.
    // Using the current time ensures a different sequence of random numbers on each run.
    srand(time(NULL));
    
    printf("Generating 5 random numbers greater than 1000:\n");
    for (int i = 0; i < 5; ++i) {
        printf("Test Case %d: %d\n", i + 1, generateRandomNumber());
    }
    
    return 0;
}