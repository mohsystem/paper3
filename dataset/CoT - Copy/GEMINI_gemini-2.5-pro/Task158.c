#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

// The following headers are for the POSIX API (fork, exec, waitpid).
// This code will only compile and run on POSIX-compliant systems like Linux and macOS.
#include <unistd.h>
#include <sys/wait.h>

// Note: A secure implementation for Windows would require the Windows-specific
// CreateProcess() API instead of fork/exec.

/**
 * Validates that the input contains only characters that are safe for a hostname or IP address.
 * This is a critical defense-in-depth measure that uses an "allow-list" approach
 * to prevent characters that could be used for command injection (e.g., ';', '|', '&', '$').
 * @param input The string to validate.
 * @return true if the input is valid, false otherwise.
 */
bool isValidInput(const char* input) {
    if (input == NULL || *input == '\0') {
        return false;
    }
    for (size_t i = 0; input[i] != '\0'; i++) {
        // Allow alphanumeric characters, dots, and hyphens.
        if (!isalnum((unsigned char)input[i]) && input[i] != '.' && input[i] != '-') {
            return false;
        }
    }
    return true;
}

/**
 * Executes an OS command (ping) securely using user-provided input.
 * It uses the POSIX fork/exec model, which passes arguments separately and
 * is the standard, secure way to avoid command injection on UNIX-like systems.
 * @param userInput The destination (hostname or IP) to be pinged.
 */
void secureOsCommand(const char* userInput) {
    printf("--- Executing command for input: '%s' ---\n", userInput);

    if (!isValidInput(userInput)) {
        printf("Invalid input format. Aborting command execution.\n");
        printf("-------------------------------------------------\n");
        return;
    }

    pid_t pid = fork();

    if (pid == -1) {
        // Fork failed
        perror("fork");
    } else if (pid == 0) {
        // --- This is the child process ---
        // execlp replaces the current process with a new one (ping).
        // Arguments are passed as a list of separate strings, preventing shell interpretation.
        // The list must be terminated by a NULL pointer.
        execlp("ping", "ping", "-c", "4", userInput, (char *)NULL);

        // If execlp returns, it means an error occurred.
        perror("execlp");
        _exit(EXIT_FAILURE); // Use _exit in child to prevent flushing parent's stdio buffers
    } else {
        // --- This is the parent process ---
        int status;
        // Wait for the child process to terminate
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            printf("\nExited with code: %d\n", WEXITSTATUS(status));
        } else {
            printf("\nChild process terminated abnormally.\n");
        }
    }
    printf("-------------------------------------------------\n");
}

int main() {
    const char* testCases[] = {
        "127.0.0.1",              // Valid IP address
        "google.com",             // Valid hostname
        "invalid-hostname!",      // Invalid character, should be rejected by validation
        "127.0.0.1; ls",          // Classic command injection, should be rejected by validation
        "$(reboot)",              // Command substitution, should be rejected by validation
    };
    
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < numTestCases; i++) {
        secureOsCommand(testCases[i]);
    }

    return 0;
}