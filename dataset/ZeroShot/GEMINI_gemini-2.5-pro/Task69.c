#define _XOPEN_SOURCE 700 // For realpath on POSIX systems
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <unistd.h>  // For rmdir
#include <limits.h>  // For PATH_MAX

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

/**
 * @brief Securely deletes a file after path validation.
 *
 * This function prevents path traversal attacks by ensuring the file's canonical path
 * is within the current working directory. It also checks that the path
 * points to a regular file and not a directory.
 * NOTE: This implementation uses POSIX-specific functions (`realpath`, `stat`).
 *
 * @param filePath The path string of the file to delete.
 * @return true if deletion was successful, false otherwise.
 */
bool deleteFileSecurely(const char* filePath) {
    if (filePath == NULL || filePath[0] == '\0') {
        fprintf(stderr, "Error: File path is null or empty.\n");
        return false;
    }

    char fileCanonicalPath[PATH_MAX];
    char cwdCanonicalPath[PATH_MAX];

    // Security Check 1: Get the canonical path of the current working directory.
    if (realpath(".", cwdCanonicalPath) == NULL) {
        perror("Error: Could not resolve current working directory path");
        return false;
    }

    // Security Check 2: Get the canonical path of the target file.
    // If the file doesn't exist, realpath returns NULL.
    if (realpath(filePath, fileCanonicalPath) == NULL) {
        // This also serves as our existence check.
        fprintf(stderr, "Error: File does not exist or path is invalid: %s\n", filePath);
        return false;
    }

    // Security Check 3: Ensure the file's path is within the CWD.
    size_t cwdLen = strlen(cwdCanonicalPath);
    if (strncmp(fileCanonicalPath, cwdCanonicalPath, cwdLen) != 0) {
        fprintf(stderr, "Error: Path traversal attempt detected. Cannot delete files outside the current directory.\n");
        return false;
    }
    // Ensure it's not the CWD itself and is in a subdirectory.
    if (fileCanonicalPath[cwdLen] != '\0' && fileCanonicalPath[cwdLen] != '/') {
        fprintf(stderr, "Error: Path traversal attempt detected (invalid subpath).\n");
        return false;
    }


    // Validation Check: Use stat to ensure it's a regular file.
    struct stat statbuf;
    if (stat(filePath, &statbuf) != 0) {
        perror("Error: Could not stat file");
        return false;
    }

    if (!S_ISREG(statbuf.st_mode)) {
        fprintf(stderr, "Error: Path does not point to a regular file (it might be a directory): %s\n", filePath);
        return false;
    }

    // Attempt to delete the file
    if (remove(filePath) == 0) {
        printf("Successfully deleted file: %s\n", filePath);
        return true;
    } else {
        perror("Error: Failed to delete file");
        return false;
    }
}

void setupTests() {
    // Create a safe file to delete
    FILE *f = fopen("test_safe.txt", "w");
    if (f) {
        fclose(f);
    }
    // Create a directory to test against
    mkdir("test_dir", 0755);
}

void cleanupTests() {
    remove("test_safe.txt"); // May have been deleted by test
    rmdir("test_dir");
}

void runTestCases() {
    printf("\n--- Running C Test Cases ---\n");
    setupTests();

    // Test 1: Success case
    printf("\n[Test 1] Deleting a safe local file...\n");
    deleteFileSecurely("./test_safe.txt");

    // Test 2: Failure case - non-existent file
    printf("\n[Test 2] Deleting a non-existent file...\n");
    deleteFileSecurely("nonexistent.txt");

    // Test 3: Failure case - directory
    printf("\n[Test 3] Deleting a directory...\n");
    deleteFileSecurely("./test_dir");

    // Test 4: Failure case - path traversal
    printf("\n[Test 4] Attempting path traversal deletion...\n");
    deleteFileSecurely("../test_unsafe.txt");
    
    // Test 5: Failure case - absolute path
    printf("\n[Test 5] Attempting to delete file with an absolute path...\n");
    char abs_path[PATH_MAX];
    snprintf(abs_path, sizeof(abs_path), "%s/abs_test.txt", P_tmpdir);
    FILE *tmp_f = fopen(abs_path, "w");
    if(tmp_f) {
        fclose(tmp_f);
        deleteFileSecurely(abs_path);
        remove(abs_path); // Cleanup
    } else {
        printf("Skipped absolute path test: could not create temp file.\n");
    }

    cleanupTests();
    printf("\nTest environment cleaned up.\n");
}


int main(int argc, char* argv[]) {
    if (argc > 1) {
        printf("Attempting to delete file from command line argument: %s\n", argv[1]);
        deleteFileSecurely(argv[1]);
    } else {
        printf("No command line arguments provided. Running test cases...\n");
        runTestCases();
    }
    return 0;
}