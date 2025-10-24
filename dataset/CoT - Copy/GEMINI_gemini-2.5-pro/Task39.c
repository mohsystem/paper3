#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/**
 * Validates a filename to ensure it is safe.
 * It checks for path traversal sequences and ensures the filename only
 * contains characters from a safe whitelist.
 *
 * @param filename The filename to validate.
 * @return True if the filename is safe, false otherwise.
 */
bool isFilenameSafe(const char* filename) {
    if (filename == NULL) {
        return false;
    }
    // Check for path traversal
    if (strstr(filename, "..") != NULL) {
        return false;
    }
    // Whitelist allowed characters: alphanumeric, dot, underscore, hyphen.
    // This ensures no shell metacharacters or directory separators are present.
    const char* whitelist = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789._-";
    for (size_t i = 0; i < strlen(filename); ++i) {
        if (strchr(whitelist, filename[i]) == NULL) {
            return false; // Character not in whitelist
        }
    }
    return true;
}

/**
 * Displays the contents of a file using the system() call.
 * CRITICAL: This function relies heavily on the isFilenameSafe() validation
 * to prevent command injection, as system() is inherently dangerous.
 *
 * @param filename The name of the file to display.
 */
void displayFileContents(const char* filename) {
    printf("--- Attempting to display file: %s ---\n", filename);

    // Security Check 1: Input validation is the primary defense.
    if (!isFilenameSafe(filename)) {
        printf("Error: Invalid or unsafe filename provided.\n");
        printf("--------------------------------------------------\n\n");
        return;
    }

    // Security Check 2: Verify file existence.
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: File does not exist or cannot be opened.\n");
        printf("--------------------------------------------------\n\n");
        return;
    }
    fclose(file);

    char command[512];
    int result;

#ifdef _WIN32
    // On Windows, use "type" and double quotes. snprintf prevents buffer overflows.
    snprintf(command, sizeof(command), "type \"%s\"", filename);
#else
    // On POSIX, use "cat" and single quotes.
    snprintf(command, sizeof(command), "cat '%s'", filename);
#endif

    printf("> Executing command: %s\n", command);
    result = system(command);

    if (result != 0) {
        fprintf(stderr, "> Command may have failed with exit code: %d\n", result);
    }
    printf("--------------------------------------------------\n\n");
}

void createTestFile(const char* name, const char* content) {
    FILE *fp = fopen(name, "w");
    if (fp != NULL) {
        fputs(content, fp);
        fclose(fp);
    }
}

int main() {
    // Setup: Create some test files
    createTestFile("test1.txt", "This is a safe test file.");
    createTestFile("safe-file.log", "This is another safe log file.");

    // Test Case 1: A valid and safe filename
    displayFileContents("test1.txt");

    // Test Case 2: Another valid filename with a hyphen
    displayFileContents("safe-file.log");

    // Test Case 3: A file that does not exist
    displayFileContents("nonexistent.txt");

    // Test Case 4: A malicious input attempting command injection
    // This is blocked by isFilenameSafe().
    displayFileContents("test1.txt; ls");

    // Test Case 5: A malicious input attempting path traversal
    // This is blocked by isFilenameSafe().
    displayFileContents("../some_other_file");

    // Cleanup: Delete the test files
    remove("test1.txt");
    remove("safe-file.log");

    return 0;
}