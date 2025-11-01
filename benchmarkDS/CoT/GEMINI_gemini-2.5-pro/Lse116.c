#include <stdio.h>
#include <stdlib.h> // For system()
#include <string.h>

#if defined(__linux__) || defined(__APPLE__)
#include <sys/wait.h> // For WIFEXITED, WEXITSTATUS
#endif

/**
 * Executes the 'ls' command with the given arguments.
 * This function uses snprintf (a "print" function) to create the command string
 * and the system() function to execute it.
 *
 * @param args A string containing the arguments for the 'ls' command.
 * @return The exit code of the command.
 */
int executeLsCommand(const char* args) {
    char command_buffer[512];

    // The snprintf function is used to create the string that is passed to system.
    // It takes a format string and a variable number of arguments.
    snprintf(command_buffer, sizeof(command_buffer), "ls %s", args);

    printf("Executing: %s\n", command_buffer);

    // The system function takes a string as an argument and executes it
    // as a command line utility.
    int exit_code = system(command_buffer);
    
    // The return value of system() is implementation-defined, but on POSIX systems,
    // we can use WEXITSTATUS to get the actual exit code.
    #if defined(__linux__) || defined(__APPLE__)
        if (WIFEXITED(exit_code)) {
            return WEXITSTATUS(exit_code);
        }
        return -1; // Indicate that it terminated abnormally
    #else
        return exit_code; // For other systems like Windows
    #endif
}

int main() {
    const char* testCases[] = {
        "-l",
        "-a .",
        "/dev/null",
        "/tmp",
        "-lh /"
    };
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < numTestCases; i++) {
        printf("--- Test Case %d ---\n", i + 1);
        printf("Arguments: '%s'\n", testCases[i]);
        int exitCode = executeLsCommand(testCases[i]);
        printf("\nExit Code: %d\n", exitCode);
        printf("--- End Test Case %d ---\n\n", i + 1);
    }

    return 0;
}