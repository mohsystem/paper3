#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * The "print function" (snprintf) is used to create the string
 * that is passed to the system function (system).
 * @param format The format string for the command (e.g., "ls %s").
 * @param arg The single argument to be formatted into the string.
 */
void execute_command(const char* format, const char* arg) {
    char command[256];
    
    // Use snprintf to safely create the command string.
    if (arg != NULL && strlen(arg) > 0) {
        snprintf(command, sizeof(command), format, arg);
    } else {
        // Handle case with no arguments by copying the format string directly.
        snprintf(command, sizeof(command), "%s", format);
    }

    printf("--- Executing command: %s ---\n", command);
    
    // The system function executes the command string.
    // The output is printed directly to the console by the command itself.
    int return_code = system(command);
    
    printf("Exited with code: %d\n", return_code);
    printf("-----------------------------------------\n\n");
}

/**
 * Main function with 5 test cases.
 */
int main() {
    // Test Case 1: simple ls
    execute_command("ls", "");

    // Test Case 2: ls with -l flag
    execute_command("ls %s", "-l");

    // Test Case 3: ls with -a flag
    execute_command("ls %s", "-a");

    // Test Case 4: ls a specific existing path
    execute_command("ls %s", ".");

    // Test Case 5: ls a non-existent path (should produce an error)
    execute_command("ls %s", "/non_existent_directory_xyz123");

    return 0;
}