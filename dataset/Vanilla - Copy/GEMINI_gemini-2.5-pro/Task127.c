#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// POSIX-specific headers for directory manipulation in main().
// For Windows, you would use <direct.h> for _mkdir and _rmdir.
#include <sys/stat.h>
#include <unistd.h> // For rmdir

/**
 * @brief Copies a source file to a destination file, handling I/O errors.
 * @param sourcePath Path to the source file.
 * @param destPath Path to the destination file.
 */
void copyFile(const char* sourcePath, const char* destPath) {
    FILE *inFile = NULL;
    FILE *outFile = NULL;
    int success = 0;

    // Open source file for reading
    inFile = fopen(sourcePath, "rb"); // Open in binary mode for portability
    if (inFile == NULL) {
        fprintf(stderr, "Error: Could not open source file '%s': %s\n", sourcePath, strerror(errno));
        goto cleanup;
    }

    // Open destination file for writing
    outFile = fopen(destPath, "wb"); // Open in binary mode
    if (outFile == NULL) {
        fprintf(stderr, "Error: Could not open destination file '%s': %s\n", destPath, strerror(errno));
        goto cleanup;
    }

    // Copy content
    int c;
    while ((c = fgetc(inFile)) != EOF) {
        if (fputc(c, outFile) == EOF) {
            fprintf(stderr, "Error: Could not write to destination file '%s': %s\n", destPath, strerror(errno));
            goto cleanup;
        }
    }

    if (ferror(inFile)) {
        fprintf(stderr, "Error: Could not read from source file '%s': %s\n", sourcePath, strerror(errno));
        goto cleanup;
    }

    printf("Success: File copied from %s to %s\n", sourcePath, destPath);
    success = 1;

cleanup:
    if (inFile) fclose(inFile);
    if (outFile) fclose(outFile);
    if (!success && destPath) remove(destPath); // Remove partial file on failure
}

int main() {
    // Note: This test setup is POSIX-specific (Linux, macOS).
    const char* testDir = "c_test_files";
    const char* validInputFile = "c_test_files/input.txt";
    const char* validOutputFile = "c_test_files/output.txt";
    const char* readOnlyDir = "c_test_files/read_only_dir";

    // Setup: Create test directory and file
    mkdir(testDir, 0755);
    FILE* f = fopen(validInputFile, "w");
    if (f) {
        fprintf(f, "This is a test file for C.\nIt has multiple lines.\n");
        fclose(f);
    }
    mkdir(readOnlyDir, 0555); // r-xr-xr-x

    printf("--- Running C Test Cases ---\n");

    // Test Case 1: Successful file copy
    printf("\n[Test Case 1: Successful Copy]\n");
    copyFile(validInputFile, validOutputFile);

    // Test Case 2: Input file not found
    printf("\n[Test Case 2: Input File Not Found]\n");
    copyFile("c_test_files/non_existent.txt", validOutputFile);

    // Test Case 3: Read error (trying to read a directory)
    printf("\n[Test Case 3: Read Error (Reading a directory)]\n");
    copyFile(testDir, validOutputFile);

    // Test Case 4: Write error (permission denied)
    printf("\n[Test Case 4: Write Error (Permission Denied)]\n");
    copyFile(validInputFile, "c_test_files/read_only_dir/output.txt");

    // Test Case 5: Write error (invalid path, directory does not exist)
    printf("\n[Test Case 5: Write Error (Invalid Path)]\n");
    copyFile(validInputFile, "c_test_files/invalid_dir/output.txt");

    // Cleanup
    chmod(readOnlyDir, 0755);
    remove("c_test_files/read_only_dir/output.txt");
    rmdir(readOnlyDir);
    remove(validInputFile);
    remove(validOutputFile);
    rmdir(testDir);
    printf("\n--- C Tests Complete ---\n");

    return 0;
}