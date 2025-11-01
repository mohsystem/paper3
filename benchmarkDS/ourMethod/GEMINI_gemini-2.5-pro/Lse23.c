#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/**
 * @brief Generates three random integers larger than 1000, calculates their sum,
 * and prints the results.
 */
void generateAndPrintRandomSum() {
    // Generate three random integers larger than 1000.
    // We'll generate numbers in the range [1001, 9999].
    // Note: rand() is not a high-quality or secure random number generator,
    // and the modulo operator can introduce bias. It is used here for
    // simplicity and portability in a non-security-critical context.
    // Range size = 9999 - 1001 + 1 = 8999
    int a = (rand() % 8999) + 1001;
    int b = (rand() % 8999) + 1001;
    int c = (rand() % 8999) + 1001;

    // Add the three integers.
    // Using long is a good practice for sums to avoid potential overflow,
    // although an int would suffice for the chosen range.
    long sum = (long)a + b + c;

    // Print the results in the specified format.
    printf("a : %d b : %d c : %d sum: %ld sum2: %ld\n", a, b, c, sum, sum);
}

/**
 * @brief Main function to run 5 test cases.
 * @return int Exit code.
 */
int main(void) {
    // Seed the random number generator once with the current time.
    // Seeding with time() is predictable and not suitable for security purposes.
    srand((unsigned int)time(NULL));

    // Run 5 test cases as requested.
    for (int i = 0; i < 5; ++i) {
        generateAndPrintRandomSum();
    }

    return 0;
}