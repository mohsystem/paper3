#include <stdio.h>

/**
 * Deletes a file at the given file path.
 *
 * @param filePath The path of the file to delete.
 * @return 0 on success, non-zero on failure.
 */
int deleteFile(const char* filePath) {
    return remove(filePath);
}

/**
 * Helper function to create an empty file for testing.
 * @param filename The name of the file to create.
 * @return 0 on success, -1 on failure.
 */
int create_dummy_file(const char* filename) {
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        return -1; // Failure
    }
    fclose(fp);
    return 0; // Success
}

/**
 * Creates and runs 5 test cases for the deleteFile function.
 */
void runTestCases() {
    printf("No command line arguments provided. Running test cases...\n");
    const char* testFiles[] = {"test1.tmp", "test2.tmp", "test3.tmp", "test4.tmp", "test5.tmp"};
    int numTests = sizeof(testFiles) / sizeof(testFiles[0]);

    // Test Cases 1-5: Create and then delete a file.
    for (int i = 0; i < numTests; i++) {
        const char* fileName = testFiles[i];
        printf("\n--- Test Case %d: Delete %s ---\n", i + 1, fileName);

        // 1. Create a dummy file for the test.
        if (create_dummy_file(fileName) != 0) {
            fprintf(stderr, "  [Setup] Error: Could not create temporary file: %s\n", fileName);
            continue; // Skip test if setup fails
        }
        printf("  [Setup] Created temporary file: %s\n", fileName);

        // 2. Attempt to delete the file.
        if (deleteFile(fileName) == 0) {
            printf("  [Result] SUCCESS: File deleted.\n");
        } else {
            printf("  [Result] FAILED: File not deleted.\n");
        }
    }
}

int main(int argc, char* argv[]) {
    // If a command-line argument is provided, use it as the file path.
    if (argc > 1) {
        const char* filePath = argv[1];
        printf("Attempting to delete file from command line: %s\n", filePath);
        if (deleteFile(filePath) == 0) {
            printf("File deleted successfully.\n");
        } else {
            fprintf(stderr, "Failed to delete the file. It may not exist or you may not have permission.\n");
        }
    } else {
        // If no arguments, run the built-in test cases.
        runTestCases();
    }
    return 0;
}