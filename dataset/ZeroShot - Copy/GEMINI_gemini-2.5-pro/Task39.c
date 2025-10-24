#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// The following headers are for POSIX systems (Linux, macOS, etc.)
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/**
 * @brief Securely displays a file's content using a system command.
 * Note: On Windows, the command "cat" might need to be changed to "type".
 * This implementation is POSIX-specific due to fork() and execlp().
 * @param filename The name of the file to display.
 */
void displayFileContent(const char* filename) {
    printf("--- Attempting to display: '%s' ---\n", filename);

    // Using fork() and one of the exec() family of functions is the secure way to
    // run external commands on POSIX systems. The filename is passed as a distinct
    // argument to the new process and is not interpreted by a shell, which prevents
    // command injection.
    pid_t pid = fork();

    if (pid == -1) {
        // Fork failed
        perror("fork failed");
        return;
    } else if (pid == 0) {
        // This is the child process.
        // execlp searches for the command in PATH. The arguments are passed separately:
        // command, arg0 (conventionally the command name), arg1 (the filename),
        // and a NULL terminator.
        execlp("cat", "cat", filename, (char*)NULL);

        // If execlp returns, it means an error occurred.
        perror("execlp failed");
        exit(EXIT_FAILURE);
    } else {
        // This is the parent process.
        int status;
        waitpid(pid, &status, 0); // Wait for the child process to complete.
        if (WIFEXITED(status)) {
            printf("-> Command finished with exit code: %d\n", WEXITSTATUS(status));
        } else {
            printf("-> Command terminated abnormally\n");
        }
    }
    printf("--- End of attempt ---\n\n");
}

/**
 * @brief Helper function to create a file with specific content.
 */
void create_test_file(const char* filename, const char* content) {
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) {
        perror("Failed to create test file");
        exit(EXIT_FAILURE);
    }
    fprintf(fp, "%s", content);
    fclose(fp);
}

int main(void) {
    // 1. Setup: Create dummy files for testing
    const char* file1 = "test_file_1.txt";
    const char* file2 = "file with spaces.txt";
    create_test_file(file1, "Hello from test file 1.");
    create_test_file(file2, "This file has spaces in its name.");
    printf("Created test files.\n\n");

    // 2. Test cases
    const char* testCases[] = {
        file1,                                // 1. Valid file
        file2,                                // 2. Valid file with spaces
        "non_existent_file.txt",              // 3. Non-existent file
        "test_file_1.txt; ls -l",             // 4. Command injection attempt (benign)
        "| whoami",                           // 5. Command injection attempt (malicious)
    };
    int num_tests = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < num_tests; ++i) {
        displayFileContent(testCases[i]);
    }

    // 3. Cleanup: Delete the dummy files
    if (remove(file1) != 0 || remove(file2) != 0) {
        perror("Error cleaning up files");
    } else {
        printf("Cleaned up test files.\n");
    }

    return 0;
}