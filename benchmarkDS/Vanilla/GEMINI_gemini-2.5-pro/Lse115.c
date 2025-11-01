#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/wait.h> // For WIFEXITED, WEXITSTATUS

/**
 * Creates a command string from a format string and a variable number of arguments,
 * writing the result to a pre-allocated buffer.
 *
 * @param buffer The character buffer to store the formatted string.
 * @param buffer_size The size of the buffer.
 * @param format The format string.
 * @param ... The arguments to be formatted into the string.
 */
void print(char* buffer, size_t buffer_size, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, buffer_size, format, args);
    va_end(args);
}

/**
 * Executes the given command line utility string.
 * It prints the standard output and standard error of the command.
 * Note: This uses popen, which is POSIX-specific.
 *
 * @param command The command string to execute.
 */
void executeLs(const char* command) {
    printf("Executing command: %s\n", command);
    FILE *pipe;
    char buffer[128];
    int exit_status;

    // To capture both stdout and stderr, we redirect stderr to stdout
    char command_with_stderr[1024];
    snprintf(command_with_stderr, sizeof(command_with_stderr), "%s 2>&1", command);

    pipe = popen(command_with_stderr, "r");
    if (pipe == NULL) {
        perror("popen failed");
        return;
    }

    // Read the combined output from the pipe and print it
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        printf("%s", buffer);
    }

    exit_status = pclose(pipe);

    if (WIFEXITED(exit_status)) {
        printf("Command exited with code: %d\n", WEXITSTATUS(exit_status));
    } else {
        fprintf(stderr, "Command terminated abnormally.\n");
    }
    printf("----------------------------------------\n");
}

int main() {
    char command_buffer[256];

    // Test Case 1: ls -l
    print(command_buffer, sizeof(command_buffer), "%s %s", "ls", "-l");
    executeLs(command_buffer);

    // Test Case 2: ls -a
    print(command_buffer, sizeof(command_buffer), "%s %s", "ls", "-a");
    executeLs(command_buffer);

    // Test Case 3: ls -lh
    print(command_buffer, sizeof(command_buffer), "%s %s", "ls", "-lh");
    executeLs(command_buffer);

    // Test Case 4: ls /tmp
    print(command_buffer, sizeof(command_buffer), "%s %s", "ls", "/tmp");
    executeLs(command_buffer);

    // Test Case 5: ls -l /
    print(command_buffer, sizeof(command_buffer), "%s %s %s", "ls", "-l", "/");
    executeLs(command_buffer);

    return 0;
}