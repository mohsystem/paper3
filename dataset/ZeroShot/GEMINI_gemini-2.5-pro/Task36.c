#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>

#define SAFE_DIRECTORY "safe_dir"

void readFileAndPrint(const char* filename) {
    // Security Check 1: Basic filename validation.
    if (filename == NULL || *filename == '\0' || strstr(filename, "/") != NULL || strstr(filename, "..") != NULL) {
        fprintf(stderr, "Error: Invalid filename provided.\n");
        return;
    }

    char fullPath[PATH_MAX];
    int len = snprintf(fullPath, sizeof(fullPath), "%s/%s", SAFE_DIRECTORY, filename);
    if (len < 0 || (size_t)len >= sizeof(fullPath)) {
        fprintf(stderr, "Error: Constructed path is too long.\n");
        return;
    }

    // Security Check 2: Canonicalize path and verify it's within the safe directory.
    char canonicalBasePath[PATH_MAX];
    if (realpath(SAFE_DIRECTORY, canonicalBasePath) == NULL) {
        fprintf(stderr, "Error: Could not resolve safe directory path: %s\n", strerror(errno));
        return;
    }

    char canonicalFilePath[PATH_MAX];
    if (realpath(fullPath, canonicalFilePath) != NULL) {
        if (strncmp(canonicalBasePath, canonicalFilePath, strlen(canonicalBasePath)) != 0) {
            fprintf(stderr, "Error: Access denied. Path is outside the safe directory.\n");
            return;
        }
    }
    // If realpath fails, it's likely because the file doesn't exist. We let 'stat' handle this.

    // Security Check 3: Ensure it's a regular file before opening.
    struct stat fileStat;
    if (stat(fullPath, &fileStat) != 0) {
        fprintf(stderr, "Error: Cannot access '%s': %s\n", filename, strerror(errno));
        return;
    }

    if (!S_ISREG(fileStat.st_mode)) {
        fprintf(stderr, "Error: '%s' is not a regular file.\n", filename);
        return;
    }

    FILE* file = fopen(fullPath, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Could not open file '%s': %s\n", filename, strerror(errno));
        return;
    }

    printf("--- Reading file: %s ---\n", filename);
    char buffer[4096];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        fwrite(buffer, 1, bytesRead, stdout);
    }
    printf("\n--- End of file: %s ---\n\n", filename);
    fclose(file);
}

void setupTestEnvironment() {
    mkdir(SAFE_DIRECTORY, 0755);
    mkdir(SAFE_DIRECTORY "/subdir", 0755);

    FILE* f;
    f = fopen(SAFE_DIRECTORY "/test1.txt", "w");
    if (f) { fputs("This is the first test file.", f); fclose(f); }

    f = fopen(SAFE_DIRECTORY "/test2.txt", "w");
    if (f) { fputs("This is the second test file.\nIt has multiple lines.", f); fclose(f); }
    
    f = fopen("insecure.txt", "w");
    if (f) { fputs("This file is outside the safe directory.", f); fclose(f); }
    
    printf("Setup complete. Safe directory is at: ./%s\n", SAFE_DIRECTORY);
    printf("Running tests...\n\n");
}

int main() {
    setupTestEnvironment();
    
    printf("Test Case 1: Reading a valid file.\n");
    readFileAndPrint("test1.txt");

    printf("Test Case 2: Reading another valid file.\n");
    readFileAndPrint("test2.txt");

    printf("Test Case 3: Attempting to read a non-existent file.\n");
    readFileAndPrint("nonexistent.txt");
    printf("\n");

    printf("Test Case 4: Path traversal attempt.\n");
    readFileAndPrint("../insecure.txt");
    printf("\n");

    printf("Test Case 5: Attempting to read a directory.\n");
    readFileAndPrint("subdir");
    printf("\n");

    return 0;
}