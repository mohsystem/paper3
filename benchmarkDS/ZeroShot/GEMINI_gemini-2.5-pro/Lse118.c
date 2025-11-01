#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

/**
 * @brief Generates 3 random floats, concatenates them, and writes to a file.
 *
 * Note on security: This function uses rand(), which is not cryptographically
 * secure. The random number generator should be seeded once using srand()
 * in the main function.
 *
 * @param filename The name of the file to write to. Avoid using untrusted
 *                 input for filenames to prevent security vulnerabilities.
 * @return 0 on success, -1 on failure.
 */
int generateAndWriteRandoms(const char* filename) {
    // 1. Generate 3 random floats.
    // rand() returns an int between 0 and RAND_MAX.
    // Cast to float and divide to get a value between 0.0 and 1.0.
    float f1 = (float)rand() / (float)RAND_MAX;
    float f2 = (float)rand() / (float)RAND_MAX;
    float f3 = (float)rand() / (float)RAND_MAX;

    // 2. Convert floats to strings and concatenate.
    // Using snprintf is crucial for security as it prevents buffer overflows.
    char buffer[128];
    int ret = snprintf(buffer, sizeof(buffer), "%f %f %f", f1, f2, f3);

    // Check if snprintf failed or if the output was truncated.
    if (ret < 0 || (size_t)ret >= sizeof(buffer)) {
        fprintf(stderr, "Error: snprintf buffer too small or encoding error.\n");
        return -1;
    }

    // 3. Write the string to a file.
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        // Always check if fopen succeeded.
        perror("Error opening file");
        return -1;
    }

    if (fprintf(fp, "%s", buffer) < 0) {
        fprintf(stderr, "Error writing to file '%s'.\n", filename);
        fclose(fp); // Clean up resource on failure.
        return -1;
    }

    // Always close the file handle to prevent resource leaks.
    if (fclose(fp) != 0) {
        perror("Error closing file");
        return -1;
    }

    return 0;
}

int main() {
    // Seed the random number generator once at the beginning of the program.
    // Using time(NULL) is a common way to get a different seed for each run.
    srand((unsigned int)time(NULL));

    printf("Running C test cases...\n");
    for (int i = 1; i <= 5; ++i) {
        // Create filename for the test case safely.
        char testFilename[64];
        snprintf(testFilename, sizeof(testFilename), "output_c_%d.txt", i);
        
        printf("Test Case %d: Writing to %s\n", i, testFilename);
        int result = generateAndWriteRandoms(testFilename);
        if (result == 0) {
            printf(" -> Success.\n");
        } else {
            printf(" -> Failed.\n");
        }
    }

    return 0;
}