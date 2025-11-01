#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Executes the 'ls' command with up to two arguments by building a command
 * string and using the system() function.
 *
 * SECURITY WARNING: This function demonstrates the pattern of using a formatting
 * function (snprintf) to build a command string for system(). This pattern is ONLY
 * secure because the arguments passed in the main function are hardcoded,
 * trusted string literals. If `arg1` or `arg2` came from an external or
 * untrusted source, this code would be highly vulnerable to command injection.
 * For example, an `arg1` of "; rm -rf /" could be catastrophic.
 * In scenarios with untrusted input, it is essential to either rigorously
 * sanitize the input or use safer APIs like the exec family of functions
 * (e.g., execvp) which do not invoke a shell.
 *
 * @param arg1 The first argument to the 'ls' command. Use "" for no argument.
 * @param arg2 The second argument to the 'ls' command. Use "" for no argument.
 */
void executeLs(const char* arg1, const char* arg2) {
    char command[512]; // A buffer to hold the constructed command.

    // Use snprintf to securely format the string, preventing buffer overflows.
    // This builds the command string as requested by the prompt.
    int ret = snprintf(command, sizeof(command), "ls %s %s", arg1, arg2);
    
    if (ret < 0 || (size_t)ret >= sizeof(command)) {
        fprintf(stderr, "Error: Failed to format command string or buffer too small.\n");
        return;
    }

    printf("Executing: %s\n", command);
    printf("------------------------------------\n");
    
    // Execute the command using system().
    int return_code = system(command);

    printf("------------------------------------\n");
    printf("Exited with code: %d\n\n", return_code);
}

int main() {
    printf("--- Test Case 1: ls -l . ---\n");
    executeLs("-l", ".");

    printf("--- Test Case 2: ls -a / ---\n");
    executeLs("-a", "/");

    printf("--- Test Case 3: ls (no arguments) ---\n");
    executeLs("", ""); // Pass empty strings for no arguments

    printf("--- Test Case 4: ls /tmp ---\n");
    executeLs("/tmp", "");

    printf("--- Test Case 5: ls /non_existent_directory_xyz ---\n");
    executeLs("/non_existent_directory_xyz", "");

    return 0;
}