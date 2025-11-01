#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/**
 * Generates 3 random floats, concatenates them into a single string,
 * and writes the string to a specified file.
 *
 * @param filename The name of the file to write to.
 */
void generate_and_write_to_file(const char* filename) {
    // 1. Generate 3 random floats
    // Note: Seeding is done once in main() for better randomness.
    float f1 = (float)rand() / (float)RAND_MAX;
    float f2 = (float)rand() / (float)RAND_MAX;
    float f3 = (float)rand() / (float)RAND_MAX;

    // 2. Convert and 3. Concatenate into a single string
    // Allocate a buffer large enough. 50 chars per float is very safe.
    char buffer[151]; 
    // Use snprintf for safe, bounded string formatting to prevent buffer overflows.
    int ret = snprintf(buffer, sizeof(buffer), "%f%f%f", f1, f2, f3);
    
    // Security check: ensure snprintf did not truncate or fail.
    if (ret < 0 || ret >= sizeof(buffer)) {
        fprintf(stderr, "Error: snprintf buffer overflow or encoding error.\n");
        return;
    }

    // 4. Write the string to a file
    FILE *file = fopen(filename, "w");
    // Security: Always check the return value of fopen to handle file open errors.
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    fprintf(file, "%s", buffer);

    // Security: Always close the file handle to prevent resource leaks.
    fclose(file);
    
    printf("Successfully wrote to %s\n", filename);
}

int main() {
    // Seed the random number generator once at the start of the program.
    srand(time(NULL));

    // Run 5 test cases
    printf("--- C Test Cases ---\n");
    for (int i = 1; i <= 5; ++i) {
        char filename[50];
        // Use snprintf to safely construct the filename string
        snprintf(filename, sizeof(filename), "c_output_%d.txt", i);
        printf("Running test case %d...\n", i);
        generate_and_write_to_file(filename);
        printf("--------------------\n");
    }

    return 0;
}