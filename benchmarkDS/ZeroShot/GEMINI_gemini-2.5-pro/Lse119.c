#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Define a safe buffer size for the strings.
#define BUFFER_SIZE 50

/**
 * Generates three random floats and converts them to strings using snprintf.
 *
 * The `rand()` function is used for random number generation. It should be
 * seeded once with `srand()` in the main function.
 *
 * For security, `snprintf` is used instead of `sprintf` to prevent buffer
 * overflows by ensuring that no more than `bufferSize` characters are written.
 *
 * @param str1 Pointer to the character buffer for the first string.
 * @param str2 Pointer to the character buffer for the second string.
 * @param str3 Pointer to the character buffer for the third string.
 * @param bufferSize The size of each character buffer.
 */
void generateRandomFloatStrings(char* str1, char* str2, char* str3, size_t bufferSize) {
    // Generate floats between 0.0 and 1.0
    float f1 = (float)rand() / (float)RAND_MAX;
    float f2 = (float)rand() / (float)RAND_MAX;
    float f3 = (float)rand() / (float)RAND_MAX;

    // Use snprintf for secure string formatting. It prevents buffer overflows.
    // It returns the number of characters that would have been written,
    // which can be checked for truncation, but is omitted here for brevity.
    snprintf(str1, bufferSize, "%f", f1);
    snprintf(str2, bufferSize, "%f", f2);
    snprintf(str3, bufferSize, "%f", f3);
}

int main() {
    // Seed the random number generator once at the start of the program.
    // Using time(NULL) makes the seed different on each run.
    srand((unsigned int)time(NULL));

    printf("Running 5 test cases...\n\n");

    // 5 test cases
    for (int i = 0; i < 5; ++i) {
        char string[BUFFER_SIZE];
        char string2[BUFFER_SIZE];
        char string3[BUFFER_SIZE];

        generateRandomFloatStrings(string, string2, string3, BUFFER_SIZE);
        
        printf("Test Case %d:\n", i + 1);
        printf("string: %s\n", string);
        printf("string2: %s\n", string2);
        printf("string3: %s\n", string3);
        printf("\n");
    }

    return 0;
}