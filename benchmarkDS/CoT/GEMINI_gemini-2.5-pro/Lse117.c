#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define BUFFER_SIZE 50

/**
 * Generates three random floats and converts them to strings.
 * The resulting strings are placed in the provided buffers.
 * @param str_a Buffer to store the first string.
 * @param str_b Buffer to store the second string.
 * @param str_c Buffer to store the third string.
 * @param buffer_size The size of each buffer to prevent overflow.
 */
void generateAndConvertRandomFloats(char *str_a, char *str_b, char *str_c, size_t buffer_size) {
    // Generate three random floats between 0.0 and 100.0
    float a = ((float)rand() / (float)RAND_MAX) * 100.0f;
    float b = ((float)rand() / (float)RAND_MAX) * 100.0f;
    float c = ((float)rand() / (float)RAND_MAX) * 100.0f;
    
    // Safely convert floats to strings using snprintf to prevent buffer overflows
    snprintf(str_a, buffer_size, "%f", a);
    snprintf(str_b, buffer_size, "%f", b);
    snprintf(str_c, buffer_size, "%f", c);
}

int main() {
    // Seed the random number generator once at the start of the program
    srand((unsigned int)time(NULL));
    
    printf("Running 5 test cases...\n");
    for (int i = 1; i <= 5; i++) {
        printf("\n--- Test Case %d ---\n", i);
        
        // Declare buffers for the strings
        char str_a[BUFFER_SIZE];
        char str_b[BUFFER_SIZE];
        char str_c[BUFFER_SIZE];
        
        generateAndConvertRandomFloats(str_a, str_b, str_c, BUFFER_SIZE);
        
        printf("str_a: %s\n", str_a);
        printf("str_b: %s\n", str_b);
        printf("str_c: %s\n", str_c);
    }
    
    return 0;
}