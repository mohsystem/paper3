#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

/**
 * Generates 3 random floats, concatenates them into a string,
 * and appends the string to the specified file.
 * @param filename The name of the file to write to.
 */
void generateAndWriteRandomFloats(const char* filename) {
    // 1. Generate 3 random floats
    // The rand() function must be seeded once, which is done in main().
    float f1 = (float)rand() / (float)RAND_MAX;
    float f2 = (float)rand() / (float)RAND_MAX;
    float f3 = (float)rand() / (float)RAND_MAX;

    // 2. Convert to strings and 3. Concatenate
    // Allocate a buffer large enough for 3 floats as strings + newline + null terminator.
    // 50 chars per float is a very safe estimate.
    char buffer[200];
    int offset = 0;
    offset += sprintf(buffer + offset, "%f", f1);
    offset += sprintf(buffer + offset, "%f", f2);
    sprintf(buffer + offset, "%f\n", f3);

    // 4. Writes the string to a file (in append mode "a")
    FILE *fp = fopen(filename, "a");
    if (fp != NULL) {
        fprintf(fp, "%s", buffer);
        fclose(fp);
    } else {
        perror("Error opening file");
    }
}

int main() {
    const char* filename = "output.c.txt";

    // Seed the random number generator once at the beginning of the program.
    srand(time(NULL));

    // Clear the file at the beginning of the run for consistent output
    FILE *fp_clear = fopen(filename, "w");
    if (fp_clear != NULL) {
        fclose(fp_clear);
    } else {
         perror("Error clearing file");
         return 1;
    }

    // Run 5 test cases
    printf("Running 5 C test cases. Writing to %s\n", filename);
    for (int i = 0; i < 5; ++i) {
        generateAndWriteRandomFloats(filename);
    }
    printf("C tests finished.\n");

    return 0;
}