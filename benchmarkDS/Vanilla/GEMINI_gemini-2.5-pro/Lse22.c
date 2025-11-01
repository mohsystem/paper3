#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/**
 * Generates a random integer greater than 1000.
 * @return An integer > 1000.
 */
int generateRandomNumber() {
    // rand() generates a number between 0 and RAND_MAX.
    // Adding 1001 ensures the result is always greater than 1000.
    // The range will be [1001, RAND_MAX + 1001].
    return 1001 + rand();
}

int main() {
    // Seed the random number generator once at the start of the program.
    srand(time(NULL));

    printf("Generating 5 random numbers greater than 1000:\n");
    for (int i = 0; i < 5; i++) {
        printf("%d\n", generateRandomNumber());
    }
    return 0;
}