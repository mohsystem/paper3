#include <stdio.h>
#include <stdlib.h> // For system()
#include <string.h> // For string manipulation

/**
 * Executes the 'ls' command with specified arguments by building a command string.
 * WARNING: This function uses system(), which executes a command in a subshell.
 * Passing unsanitized user input is extremely dangerous and can lead to command
 * injection vulnerabilities. This code also uses a fixed buffer and is susceptible
 * to buffer overflows if the combined arguments are too long. It is for
 * demonstrating the described pattern only and should NOT be used in production.
 *
 * @param argc The number of arguments in the args array.
 * @param args An array of string arguments for the 'ls' command.
 */
void executeLsCommand(int argc, const char* args[]) {
    // Using a fixed-size buffer is risky. In production code, you should
    // calculate the required size first and allocate memory dynamically.
    char command[1024];

    // Use a "print" style function (snprintf) to create the start of the command string.
    // This is safer than strcpy.
    int offset = snprintf(command, sizeof(command), "ls");

    for (int i = 0; i < argc; ++i) {
        // This is a safer way to build the string than strcat, preventing buffer overflows.
        // It keeps track of the remaining space in the buffer.
        int remaining_space = sizeof(command) - offset;
        if (remaining_space <= 1) { // Need space for at least a null terminator
            fprintf(stderr, "Warning: Command buffer full, truncating arguments.\n");
            break;
        }
        // Format " <arg>" into the remaining buffer space.
        offset += snprintf(command + offset, remaining_space, " %s", args[i]);
    }

    printf("--- Executing command: '%s' ---\n", command);
    int exit_code = system(command);
    printf("--- Command finished with exit code: %d ---\n\n", exit_code);
}

int main() {
    // Test Case 1: ls
    printf(">>> Test Case 1: ls (no arguments)\n");
    executeLsCommand(0, NULL);

    // Test Case 2: ls -l
    printf(">>> Test Case 2: ls -l\n");
    const char* args2[] = {"-l"};
    executeLsCommand(1, args2);

    // Test Case 3: ls -a -h
    printf(">>> Test Case 3: ls -a -h\n");
    const char* args3[] = {"-a", "-h"};
    executeLsCommand(2, args3);

    // Test Case 4: ls -l /tmp
    printf(">>> Test Case 4: ls -l /tmp\n");
    const char* args4[] = {"-l", "/tmp"};
    executeLsCommand(2, args4);

    // Test Case 5: ls on a non-existent directory
    printf(">>> Test Case 5: ls /non_existent_dir_12345\n");
    const char* args5[] = {"/non_existent_dir_12345"};
    executeLsCommand(1, args5);

    return 0;
}