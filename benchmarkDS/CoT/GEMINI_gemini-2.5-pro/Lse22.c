#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

/**
 * @brief Generates a random integer greater than 1000.
 *
 * Note: The random number generator must be seeded using srand() once
 * in the main program before this function is called.
 *
 * @return A random integer in the range [1001, INT_MAX-1].
 */
int generateRandomNumber() {
    // We want a number > 1000, which means >= 1001.
    // The expression `rand() % N` can introduce a slight modulo bias if RAND_MAX+1
    // is not a multiple of N. For non-cryptographic uses, this is acceptable.
    // To prevent integer overflow from an expression like `rand() + 1001`
    // (since RAND_MAX can be equal to INT_MAX), we use the modulo operator.
    
    // We generate a number in the range [0, INT_MAX - 1001 - 1]
    int range = INT_MAX - 1001;
    if (range <= 0) {
        // This case should not happen on any modern system.
        return 1001;
    }
    
    int randomNumber = (rand() % range) + 1001;

    return randomNumber;
}

int main(void) {
    // Seed the random number generator using the current time.
    // This should only be done once at the beginning of the program.
    srand((unsigned int)time(NULL));
    
    printf("Generating 5 random numbers greater than 1000:\n");
    for (int i = 0; i < 5; ++i) {
        printf("Test Case %d: %d\n", i + 1, generateRandomNumber());
    }
    
    return 0;
}