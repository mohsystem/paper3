#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h> // For isalpha

#ifdef _WIN32
#include <io.h>
#define fileno _fileno
#define fstat _fstat
#define stat _stat
#else
#include <unistd.h>
#endif


// Function to check if a path is potentially unsafe.
bool isPathSafe(const char* path) {
    if (path == NULL) {
        return false;
    }
    // Disallow paths containing ".."
    if (strstr(path, "..") != NULL) {
        fprintf(stderr, "Error: Path traversal detected.\n");
        return false;
    }
    // Disallow absolute paths
    if (path[0] == '/' || path[0] == '\\') {
        fprintf(stderr, "Error: Absolute paths are not allowed.\n");
        return false;
    }
    // A simple check for Windows absolute paths like "C:\"
    if (strlen(path) > 2 && isalpha((unsigned char)path[0]) && path[1] == ':') {
        fprintf(stderr, "Error: Absolute paths are not allowed.\n");
        return false;
    }
    return true;
}

// Function to print file contents.
void printFileContents(const char* filename) {
    if (!isPathSafe(filename)) {
        return;
    }

    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("Error opening file");
        return;
    }

    // Check if the opened entity is a regular file to avoid reading special files/directories.
    // This is done *after* opening to avoid a TOCTOU vulnerability.
    struct stat file_stat;
    if (fstat(fileno(fp), &file_stat) == -1) {
        perror("Error getting file stats");
        fclose(fp);
        return;
    }

    if (!S_ISREG(file_stat.st_mode)) {
        fprintf(stderr, "Error: '%s' is not a regular file.\n", filename);
        fclose(fp);
        return;
    }

    char buffer[BUFSIZ]; // Use a standard buffer size
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        // Use fputs for safer printing (doesn't interpret format specifiers)
        fputs(buffer, stdout);
    }

    if (ferror(fp)) {
        perror("Error reading file");
    }

    fclose(fp);
}

void runTestCase(const char* testName, const char* filename, const char* content, bool createFile) {
    printf("--- %s ---\n", testName);
    printf("Attempting to read: %s\n", filename);

    if (createFile) {
        FILE* testFile = fopen(filename, "w");
        if (testFile != NULL) {
            fputs(content, testFile);
            fclose(testFile);
        } else {
            fprintf(stderr, "Failed to create test file: %s\n", filename);
            return;
        }
    }

    printFileContents(filename);

    if (createFile) {
        remove(filename);
    }
    
    printf("--------------------\n\n");
}

int main() {
    // Test Case 1: Standard file with multiple lines
    runTestCase("Test Case 1: Standard File", "test1.txt", "Hello, C World!\nThis is a test file.", true);

    // Test Case 2: Empty file
    runTestCase("Test Case 2: Empty File", "test2.txt", "", true);

    // Test Case 3: File with a single line
    runTestCase("Test Case 3: Single Line File", "test3.txt", "One line only.", true);

    // Test Case 4: Non-existent file
    runTestCase("Test Case 4: Non-Existent File", "nonexistent.txt", "", false);

    // Test Case 5: Unsafe path (directory traversal)
    runTestCase("Test Case 5: Unsafe Path", "../unsafe.txt", "This should not be accessed.", false);
    
    return 0;
}