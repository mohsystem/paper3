#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Reads a file specified by the file path and prints its contents to the console.
 * Handles file opening errors gracefully.
 *
 * @param filePath The path to the file to be read.
 */
void readFileAndPrint(const char* filePath) {
    // Security: Basic input validation to prevent segfaults from NULL pointers
    // or issues with empty strings.
    if (filePath == NULL || filePath[0] == '\0') {
        fprintf(stderr, "Error: File path cannot be null or empty.\n");
        return;
    }

    FILE* file = fopen(filePath, "r");

    // Security: Always check the return value of fopen. If it's NULL,
    // the file could not be opened. Using perror() provides a descriptive
    // system error message (e.g., "No such file or directory", "Permission denied").
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    // Security: Use a fixed-size buffer and read with fgets() instead of gets()
    // to prevent buffer overflows. fgets() reads up to size-1 characters.
    char line[256];
    while (fgets(line, sizeof(line), file) != NULL) {
        printf("%s", line);
    }

    // Security: It is crucial to close the file handle to prevent resource leaks.
    fclose(file);
}

int main() {
    printf("--- C Test Cases ---\n");
    const char* testFileName = "c_test_file.txt";

    // Setup: Create a temporary file for testing
    FILE* testFile = fopen(testFileName, "w");
    if (testFile == NULL) {
        perror("Failed to create test file");
        return 1;
    }
    fprintf(testFile, "Hello from the C test file.\n");
    fprintf(testFile, "This is the second line.\n");
    fprintf(testFile, "End of file.\n");
    fclose(testFile);

    // Test Case 1: Read a valid, existing file.
    printf("\n[Test Case 1: Reading a valid file]\n");
    readFileAndPrint(testFileName);

    // Test Case 2: Attempt to read a non-existent file.
    printf("\n[Test Case 2: Reading a non-existent file]\n");
    readFileAndPrint("non_existent_file.txt");

    // Test Case 3: Attempt to read with a NULL path.
    printf("\n[Test Case 3: Reading a NULL path]\n");
    readFileAndPrint(NULL);

    // Test Case 4: Attempt to read with an empty path.
    printf("\n[Test Case 4: Reading an empty path]\n");
    readFileAndPrint("");

    // Test Case 5: Attempt to read a directory.
    printf("\n[Test Case 5: Reading a directory]\n");
    readFileAndPrint("."); // "." represents the current directory

    // Cleanup: Delete the temporary test file
    if (remove(testFileName) != 0) {
        perror("Failed to delete test file");
    }

    return 0;
}