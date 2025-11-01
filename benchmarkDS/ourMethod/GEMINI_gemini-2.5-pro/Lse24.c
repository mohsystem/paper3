#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/**
 * Generates two random integers within a specified range, calculates their difference,
 * and prints the results.
 *
 * @param min The minimum value of the random number range (inclusive).
 * @param max The maximum value of the random number range (inclusive).
 */
void generateAndCalculateDifference(int min, int max) {
    if (min > max) {
        fprintf(stderr, "Error: min value cannot be greater than max value.\n");
        return;
    }

    // The expression rand() % N gives a number in [0, N-1].
    // We want a range of size (max - min + 1).
    int num1 = (rand() % (max - min + 1)) + min;

    // Generate another random integer between min and max.
    int num2 = (rand() % (max - min + 1)) + min;

    // Calculate the difference between the two integers.
    int difference = num1 - num2;

    // Print the results.
    printf("First random number: %d\n", num1);
    printf("Second random number: %d\n", num2);
    printf("Difference (%d - %d): %d\n", num1, num2, difference);
}

int main(void) {
    const int MIN_VAL = 10;
    const int MAX_VAL = 1000;

    // Seed the random number generator once at the start of the program.
    // Using the current time ensures different random numbers on each run.
    srand((unsigned int)time(NULL));

    printf("Running 5 test cases...\n");
    for (int i = 0; i < 5; ++i) {
        printf("\nTest Case %d:\n", i + 1);
        generateAndCalculateDifference(MIN_VAL, MAX_VAL);
    }

    return 0;
}