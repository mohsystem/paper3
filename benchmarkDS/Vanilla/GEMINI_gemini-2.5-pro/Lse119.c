#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/**
 * Generates three random floats and writes their string representations
 * into the provided character buffers using sprintf.
 * @param string The buffer for the first float string.
 * @param string2 The buffer for the second float string.
 * @param string3 The buffer for the third float string.
 */
void generate_random_float_strings(char* string, char* string2, char* string3) {
    // Generate three random floats between 0.0 and 1.0
    float f1 = (float)rand() / (float)RAND_MAX;
    float f2 = (float)rand() / (float)RAND_MAX;
    float f3 = (float)rand() / (float)RAND_MAX;

    // Convert floats to strings using sprintf as requested.
    // Note: sprintf is unsafe because it doesn't check buffer sizes.
    // snprintf is the recommended, safer alternative in modern C programming.
    sprintf(string, "%f", f1);
    sprintf(string2, "%f", f2);
    sprintf(string3, "%f", f3);
}

int main() {
    // Seed the random number generator once at the start of the program
    srand(time(NULL));

    printf("C Test Cases:\n");
    for (int i = 0; i < 5; i++) {
        // Define character arrays to store the strings.
        // A size of 50 is generally sufficient for a float's string representation.
        char string1[50];
        char string2[50];
        char string3[50];
        
        generate_random_float_strings(string1, string2, string3);
        
        printf("Test Case %d: [\"%s\", \"%s\", \"%s\"]\n", i + 1, string1, string2, string3);
    }

    return 0;
}