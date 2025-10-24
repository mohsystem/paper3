#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_FILENAME_LEN 255
#define MAX_COMMAND_LEN 512

/**
 * @brief Validates a filename against a strict whitelist.
 *
 * This function ensures that the filename is safe to be used in a shell command.
 * It checks for:
 * - Non-NULL and non-empty string.
 * - Length within a reasonable limit (MAX_FILENAME_LEN).
 * - Characters are restricted to a whitelist (alphanumeric, dot, underscore, hyphen).
 * - Prevents special directory names "." and "..".
 * This helps mitigate command injection and path traversal vulnerabilities.
 *
 * @param filename The filename to validate.
 * @return true if the filename is valid, false otherwise.
 */
bool isValidFilename(const char* filename) {
    if (filename == NULL || filename[0] == '\0') {
        return false;
    }

    size_t len = strlen(filename);
    if (len > MAX_FILENAME_LEN) {
        return false;
    }

    const char* whitelist = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789._-";
    if (strspn(filename, whitelist) != len) {
        return false;
    }
    
    if (strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0) {
        return false;
    }

    return true;
}

/**
 * @brief Displays the content of a file using a system command.
 *
 * It first validates the filename to prevent security vulnerabilities before
 * constructing and executing a system command (`cat` on POSIX, `type` on Windows).
 *
 * @param filename The name of the file to display. The file should be in the current directory.
 */
void displayFileContent(const char* filename) {
    if (!isValidFilename(filename)) {
        fprintf(stderr, "Error: Invalid filename provided: \"%s\"\n\n", filename);
        return;
    }

    char command[MAX_COMMAND_LEN];
    int chars_written;

    // Construct the command safely using snprintf to prevent buffer overflows.
    // Quoting the filename adds an extra layer of defense.
#ifdef _WIN32
    chars_written = snprintf(command, sizeof(command), "type \"%s\"", filename);
#else
    chars_written = snprintf(command, sizeof(command), "cat '%s'", filename);
#endif

    if (chars_written < 0 || (size_t)chars_written >= sizeof(command)) {
        fprintf(stderr, "Error: Failed to construct command for filename: %s\n\n", filename);
        return;
    }
    
    printf("--- Executing command: %s ---\n", command);
    int result = system(command);
    if (result != 0) {
        // The shell command (cat/type) usually prints a descriptive error to stderr,
        // so we just note that the command might have failed.
        fprintf(stderr, "--- Command may have failed or file not found. Exit code: %d ---\n", result);
    }
    printf("--- End of content for %s ---\n\n", filename);
}


/**
 * @brief Helper function to create a dummy file for testing purposes.
 * @param filename The name of the file to create.
 * @param content The content to write to the file.
 */
void createTestFile(const char* filename, const char* content) {
    FILE* fp = fopen(filename, "w");
    if (fp != NULL) {
        fprintf(fp, "%s", content);
        fclose(fp);
    } else {
        perror("Failed to create test file");
    }
}


int main(void) {
    // Setup: Create a couple of files for testing.
    createTestFile("test1.txt", "This is the first test file.");
    createTestFile("test-file_2.log", "This is the second test file.\nIt has a hyphen and an underscore.");

    const char* testFilenames[] = {
        // Test Case 1: A valid, simple filename that exists.
        "test1.txt",
        // Test Case 2: A valid filename with allowed special characters that exists.
        "test-file_2.log",
        // Test Case 3: An invalid filename with a command injection attempt.
        "test.txt; ls -la",
        // Test Case 4: An invalid filename with path traversal characters.
        "../../etc/passwd",
        // Test Case 5: A non-existent but validly named file.
        "no_such_file.txt"
    };

    int num_tests = sizeof(testFilenames) / sizeof(testFilenames[0]);
    for (int i = 0; i < num_tests; ++i) {
        printf("Attempting to display: \"%s\"\n", testFilenames[i]);
        displayFileContent(testFilenames[i]);
    }

    // Cleanup: Remove the created test files.
    remove("test1.txt");
    remove("test-file_2.log");

    return 0;
}