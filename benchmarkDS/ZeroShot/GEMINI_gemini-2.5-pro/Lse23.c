#include <stdio.h>
#include <stdlib.h>
#include <time.h>    // For time() to seed rand()

/**
 * @brief Generates three pseudo-random integers greater than 1000,
 * calculates their sum safely, and prints the result.
 *
 * SECURITY WARNING: This function uses rand(), which is a pseudo-random
 * number generator (PRNG). It is NOT cryptographically secure. The sequence of
 * numbers is predictable and should not be used for security-sensitive
 * applications like cryptography, generating tokens, or gambling.
 * For secure random numbers in C, platform-specific APIs are required,
 * such as reading from /dev/urandom on Linux/macOS or using CryptGenRandom()
 * on Windows.
 */
void generateAndPrintSum() {
    // Generate three pseudo-random integers in the range [1001, 30000].
    // The expression `rand() % N` can introduce a slight bias if RAND_MAX is not
    // a multiple of N, but it is acceptable for non-critical applications.
    int a = (rand() % 29000) + 1001;
    int b = (rand() % 29000) + 1001;
    int c = (rand() % 29000) + 1001;

    // Use 'long long' for the sum to prevent signed integer overflow.
    // In C, signed integer overflow is undefined behavior, a major security risk.
    // A standard 32-bit int can hold a max value of ~2.1 billion. The sum of
    // three ints could exceed this, but will fit within a 64-bit long long.
    long long sum = (long long)a + b + c;

    // Print the numbers and their sum in the specified format.
    printf("a : %d b : %d c : %d sum: %lld sum2: %lld\n", a, b, c, sum, sum);
}

int main() {
    // Seed the random number generator. This should only be done once.
    // Seeding with the current time is a common practice, but it makes the
    // sequence predictable if the execution time is known.
    srand((unsigned int)time(NULL));

    printf("C execution:\n");
    // Run 5 test cases as requested.
    int i;
    for (i = 0; i < 5; ++i) {
        generateAndPrintSum();
    }

    return 0;
}