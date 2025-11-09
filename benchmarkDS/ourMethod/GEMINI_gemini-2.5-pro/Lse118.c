#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h>

/**
 * @brief Generates 3 random floats, concatenates them into a string, 
 *        and writes the string to a new file.
 * 
 * This function creates a new file and writes to it. It uses exclusive create
 * mode ("wx") for the file opening, which is an atomic operation that prevents 
 * race conditions (TOCTOU) and avoids overwriting existing files.
 *
 * @param filename The path to the file to be created. The function will fail 
 *                 if the file already exists. The filename must not be NULL 
 *                 or an empty string.
 * @return 0 on success, -1 on failure.
 */
int generate_and_write_floats(const char *filename) {
    // Rule #16: Input validation. Check for NULL or empty filename.
    if (filename == NULL || filename[0] == '\0') {
        fprintf(stderr, "Error: Invalid filename provided.\n");
        return -1;
    }

    // Generate 3 random floats between 0.0 and 1.0
    float f1 = (float)rand() / (float)RAND_MAX;
    float f2 = (float)rand() / (float)RAND_MAX;
    float f3 = (float)rand() / (float)RAND_MAX;

    // Buffer to hold the concatenated string of floats.
    // A standard float string representation is relatively small.
    // 128 bytes is a safe size for three floats and separators.
    char buffer[128];
    
    // Rule #36 (C-specific): Use snprintf to prevent buffer overflows.
    // It returns the number of characters that would have been written.
    int chars_written = snprintf(buffer, sizeof(buffer), "%.8f, %.8f, %.8f", f1, f2, f3);

    // Check for snprintf errors or truncation.
    // Rule #22: Check all function return values.
    if (chars_written < 0) {
        fprintf(stderr, "Error: snprintf encoding error occurred.\n");
        return -1;
    }
    if ((size_t)chars_written >= sizeof(buffer)) {
        fprintf(stderr, "Error: snprintf output was truncated. Buffer too small.\n");
        return -1;
    }

    FILE *file = NULL;

    // C-Specific Mitigation: Use "wx" mode for atomic file creation.
    // This prevents TOCTOU (Time-of-Check to Time-of-Use) race conditions
    // by ensuring the file is created and opened in a single, atomic step.
    // It fails if the file already exists. This follows Rules #6, #7, #21.
    file = fopen(filename, "wx");
    if (file == NULL) {
        // perror provides a descriptive error message from the OS (e.g., "File exists")
        // Rule #25: Do not leak internal details. perror is safe.
        perror("Error opening file for exclusive writing");
        return -1;
    }

    // Write the concatenated string to the file.
    if (fputs(buffer, file) == EOF) {
        perror("Error writing to file");
        fclose(file); // Attempt to close the file even on write error
        return -1;
    }

    // Close the file stream.
    if (fclose(file) != 0) {
        perror("Error closing file");
        return -1;
    }

    return 0;
}

int main(void) {
    // Seed the random number generator once at the beginning of the program.
    srand((unsigned int)time(NULL));

    printf("--- Running Test Cases ---\n\n");

    // Test Case 1: Successful creation
    const char* file1 = "output1.txt";
    printf("Test 1: Writing to %s\n", file1);
    // For test repeatability, remove the file if it exists from a previous run.
    // In a real application, the failure on an existing file would be desired.
    remove(file1); 
    if (generate_and_write_floats(file1) == 0) {
        printf("Test 1: Success. File '%s' created.\n", file1);
    } else {
        printf("Test 1: Failed.\n");
    }
    printf("\n");

    // Test Case 2: Another successful creation
    const char* file2 = "output2.txt";
    printf("Test 2: Writing to %s\n", file2);
    remove(file2);
    if (generate_and_write_floats(file2) == 0) {
        printf("Test 2: Success. File '%s' created.\n", file2);
    } else {
        printf("Test 2: Failed.\n");
    }
    printf("\n");
    
    // Test Case 3: Test failure on an existing file
    const char* file3 = "output3.txt";
    printf("Test 3: Attempting to write to '%s' twice.\n", file3);
    remove(file3);
    if (generate_and_write_floats(file3) == 0) {
        printf("Test 3: First write successful (as expected).\n");
        // Second attempt should fail because the file now exists.
        if (generate_and_write_floats(file3) != 0) {
            printf("Test 3: Second write failed as expected ('File exists').\n");
        } else {
            printf("Test 3: Second write succeeded (UNEXPECTED).\n");
        }
    } else {
        printf("Test 3: First write failed (UNEXPECTED).\n");
    }
    printf("\n");

    // Test Case 4: Test with NULL filename
    printf("Test 4: Calling with NULL filename.\n");
    if (generate_and_write_floats(NULL) != 0) {
        printf("Test 4: Failed as expected.\n");
    } else {
        printf("Test 4: Succeeded (UNEXPECTED).\n");
    }
    printf("\n");

    // Test Case 5: Test with empty filename
    printf("Test 5: Calling with empty filename.\n");
    if (generate_and_write_floats("") != 0) {
        printf("Test 5: Failed as expected.\n");
    } else {
        printf("Test 5: Succeeded (UNEXPECTED).\n");
    }
    printf("\n");

    printf("--- Test Cases Finished ---\n");

    return 0;
}