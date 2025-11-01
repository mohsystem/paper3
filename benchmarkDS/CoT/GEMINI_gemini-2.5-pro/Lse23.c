#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/**
 * @brief Generates three random integers larger than 1000, calculates their sum,
 *        and prints the results in a specific format.
 */
void generateAndPrintRandomSum() {
    // Generate three random integers in the range [1001, 9999].
    // The range size is 9999 - 1001 + 1 = 8999.
    // (rand() % range_size) gives a number in [0, range_size - 1].
    // We add the minimum value (1001) to shift the range.
    int a = (rand() % 8999) + 1001;
    int b = (rand() % 8999) + 1001;
    int c = (rand() % 8999) + 1001;

    // Calculate the sum. An 'int' is sufficient.
    int sum = a + b + c;

    // Print the result using printf for formatted output.
    printf("a : %d b : %d c : %d sum: %d sum2: %d\n", a, b, c, sum, sum);
}

int main() {
    // Seed the random number generator once at the beginning of the program.
    // Using the current time ensures a different sequence of random numbers on each run.
    srand(time(NULL));

    // Run 5 test cases as requested.
    printf("Running C Test Cases:\n");
    for (int i = 0; i < 5; i++) {
        generateAndPrintRandomSum();
    }

    return 0;
}