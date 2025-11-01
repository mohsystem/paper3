#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Define a sufficient buffer size for the string representation of a float.
// A standard float has about 7 decimal digits of precision.
// With sign, decimal point, exponent, etc., 50 is a very safe size to prevent buffer overflows.
#define BUFFER_SIZE 50

/**
 * @brief Generates three random floats, converts them to strings, and stores them in the provided buffers.
 * 
 * @param str_a Pointer to a character buffer to store the first string.
 * @param str_b Pointer to a character buffer to store the second string.
 * @param str_c Pointer to a character buffer to store the third string.
 */
void generateAndConvertRandomFloats(char* str_a, char* str_b, char* str_c) {
    // Generate three random floats in the range [0.0, 1.0].
    // WARNING: rand() is not cryptographically secure and its quality can be poor.
    // For security-sensitive applications, use a platform-specific API (e.g., /dev/urandom on Linux,
    // CryptGenRandom on Windows) or a dedicated cryptographic library.
    float a = (float)rand() / (float)RAND_MAX;
    float b = (float)rand() / (float)RAND_MAX;
    float c = (float)rand() / (float)RAND_MAX;

    // Convert floats to strings securely using snprintf.
    // snprintf is crucial as it prevents buffer overflows by ensuring that no more
    // than BUFFER_SIZE characters (including the null terminator) are written.
    snprintf(str_a, BUFFER_SIZE, "%f", a);
    snprintf(str_b, BUFFER_SIZE, "%f", b);
    snprintf(str_c, BUFFER_SIZE, "%f", c);
}

int main() {
    // Seed the random number generator once at the beginning of the program.
    // Using time(NULL) is common but predictable, making it unsuitable for secure applications.
    // A better seed might combine time with other sources of entropy like the process ID.
    srand((unsigned int)time(NULL));

    // Run 5 test cases to demonstrate the functionality.
    for (int i = 0; i < 5; ++i) {
        printf("Test Case %d:\n", i + 1);

        // The resulting strings are stored in these variables as per the prompt.
        char str_a[BUFFER_SIZE];
        char str_b[BUFFER_SIZE];
        char str_c[BUFFER_SIZE];

        generateAndConvertRandomFloats(str_a, str_b, str_c);

        printf("  str_a: %s\n", str_a);
        printf("  str_b: %s\n", str_b);
        printf("  str_c: %s\n", str_c);
        printf("\n");
    }

    return 0;
}