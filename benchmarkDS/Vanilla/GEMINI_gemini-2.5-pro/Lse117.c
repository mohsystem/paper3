#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Define a buffer size for the string conversion
#define BUFFER_SIZE 50

/**
 * Generates three random floats and converts them to strings.
 * The resulting strings are stored in the provided character arrays.
 * @param str_a Pointer to a character buffer to store the first string.
 * @param str_b Pointer to a character buffer to store the second string.
 * @param str_c Pointer to a character buffer to store the third string.
 */
void generateRandomFloatStrings(char* str_a, char* str_b, char* str_c) {
    // Generate three random floats between 0.0 and 100.0
    float a = ((float)rand() / (float)RAND_MAX) * 100.0f;
    float b = ((float)rand() / (float)RAND_MAX) * 100.0f;
    float c = ((float)rand() / (float)RAND_MAX) * 100.0f;

    // Convert floats to strings safely using snprintf
    snprintf(str_a, BUFFER_SIZE, "%f", a);
    snprintf(str_b, BUFFER_SIZE, "%f", b);
    snprintf(str_c, BUFFER_SIZE, "%f", c);
}

int main() {
    // Seed the random number generator once at the start of the program
    srand(time(NULL));

    for (int i = 0; i < 5; i++) {
        printf("Test Case %d:\n", i + 1);
        
        char str_a[BUFFER_SIZE];
        char str_b[BUFFER_SIZE];
        char str_c[BUFFER_SIZE];

        generateRandomFloatStrings(str_a, str_b, str_c);

        printf("  str_a: %s\n", str_a);
        printf("  str_b: %s\n", str_b);
        printf("  str_c: %s\n", str_c);
        printf("\n");
    }

    return 0;
}