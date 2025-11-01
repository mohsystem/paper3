#include <stdio.h>
#include <stdlib.h> // For rand(), srand(), RAND_MAX
#include <time.h>   // For time()

/**
 * Generates three random floats and converts them to strings,
 * storing them in the provided character buffers.
 *
 * @param string1 Pointer to the buffer for the first string.
 * @param size1 The size of the first buffer.
 * @param string2 Pointer to the buffer for the second string.
 * @param size2 The size of the second buffer.
 * @param string3 Pointer to the buffer for the third string.
 * @param size3 The size of the third buffer.
 */
void generateRandomFloatStrings(char* string1, size_t size1, 
                                char* string2, size_t size2, 
                                char* string3, size_t size3) {
    // Generate three random floats between 0.0 and 1.0
    float f1 = (float)rand() / (float)RAND_MAX;
    float f2 = (float)rand() / (float)RAND_MAX;
    float f3 = (float)rand() / (float)RAND_MAX;

    // Use snprintf to safely convert floats to strings, preventing buffer overflows.
    // This is a secure alternative to the less safe sprintf.
    snprintf(string1, size1, "%f", f1);
    snprintf(string2, size2, "%f", f2);
    snprintf(string3, size3, "%f", f3);
}

int main() {
    // Seed the random number generator. This should only be done once.
    srand((unsigned int)time(NULL));

    printf("Running 5 test cases for C:\n");
    for (int i = 0; i < 5; i++) {
        // Define character buffers to store the resulting strings.
        // A size of 50 is more than enough for a standard float representation.
        char str1[50];
        char str2[50];
        char str3[50];

        generateRandomFloatStrings(str1, sizeof(str1), str2, sizeof(str2), str3, sizeof(str3));

        printf("Test Case %d: [\"%s\", \"%s\", \"%s\"]\n", (i + 1), str1, str2, str3);
    }

    return 0;
}