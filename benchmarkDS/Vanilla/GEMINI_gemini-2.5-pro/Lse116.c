#include <stdio.h>
#include <stdlib.h> // For system()

/**
 * @brief Executes a system command.
 * 
 * @param command The command string to execute.
 * @return The exit code of the command.
 */
int executeCommand(const char* command) {
    // The system() function passes the command string to the command
    // interpreter of the host system (e.g., /bin/sh on Linux).
    int exit_code = system(command);
    return exit_code;
}

int main() {
    const char* testCommands[] = {
        "ls",
        "ls -l",
        "ls -a",
        "ls -al",
        "ls non_existent_directory"
    };
    int num_tests = sizeof(testCommands) / sizeof(testCommands[0]);

    for (int i = 0; i < num_tests; ++i) {
        // The printf function is used to create the formatted string for output.
        printf("--- Test Case %d: Executing '%s' ---\n", i + 1, testCommands[i]);
        
        int exit_code = executeCommand(testCommands[i]);
        
        // The return value of system() is implementation-defined. On POSIX systems,
        // macros like WIFEXITED and WEXITSTATUS from <sys/wait.h> should be used
        // to interpret the return value correctly. For simplicity, we just print the raw value.
        printf("--- Command exited with raw status: %d ---\n\n", exit_code);
    }

    return 0;
}