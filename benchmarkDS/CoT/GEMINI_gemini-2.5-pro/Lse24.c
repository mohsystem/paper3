#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/**
 * @brief Generates two random integers between 10 and 1000, calculates their
 * difference, and prints the results.
 */
void generateAndCalculateDifference() {
    int min = 10;
    int max = 1000;

    // Generate the first random integer between 10 and 1000 (inclusive).
    // Formula for range [min, max]: rand() % (max - min + 1) + min
    int num1 = (rand() % (max - min + 1)) + min;

    // Generate the second random integer between 10 and 1000 (inclusive).
    int num2 = (rand() % (max - min + 1)) + min;

    // Calculate the difference between the two integers.
    int difference = num1 - num2;

    // Print the results.
    printf("First number: %d\n", num1);
    printf("Second number: %d\n", num2);
    printf("Difference (First - Second): %d\n", difference);
}

int main() {
    // Seed the random number generator once at the beginning of the program
    // to ensure different results on each run.
    srand(time(NULL));

    printf("Running 5 test cases:\n");
    for (int i = 0; i < 5; ++i) {
        printf("--- Test Case %d ---\n", i + 1);
        generateAndCalculateDifference();
    }

    return 0;
}