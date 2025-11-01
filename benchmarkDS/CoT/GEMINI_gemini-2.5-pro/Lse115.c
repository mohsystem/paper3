#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/**
 * Formats a string using placeholders, similar to printf.
 * This function is safe from buffer overflows.
 * The caller is responsible for freeing the returned memory.
 */
char* customPrint(const char* format, ...) {
    va_list args1;
    va_start(args1, format);
    va_list args2;
    va_copy(args2, args1);

    // Determine required size using vsnprintf
    int size = vsnprintf(NULL, 0, format, args1);
    va_end(args1);

    if (size < 0) {
        va_end(args2);
        return NULL; // Error
    }

    // Allocate memory for the formatted string (+1 for null terminator)
    char* buffer = (char*)malloc(size + 1);
    if (!buffer) {
        va_end(args2);
        return NULL; // Allocation failed
    }

    // Actually format the string
    vsnprintf(buffer, size + 1, format, args2);
    va_end(args2);

    return buffer;
}

/**
 * Executes a command line utility and captures its output.
 * WARNING: Executing commands from strings is inherently dangerous if any part
 * of the string comes from an untrusted source, as it can lead to command
 * injection vulnerabilities. This implementation is for demonstration purposes only.
 * The caller is responsible for freeing the returned memory.
 */
char* executeCommand(const char* command) {
    FILE* pipe = popen(command, "r");
    if (!pipe) {
        perror("popen failed");
        return NULL;
    }

    size_t buffer_size = 256;
    size_t current_size = 0;
    char* result = (char*)malloc(buffer_size);
    if (!result) {
        pclose(pipe);
        return NULL; // Allocation failed
    }
    result[0] = '\0'; // Start with an empty string

    char chunk[128];
    while (fgets(chunk, sizeof(chunk), pipe) != NULL) {
        size_t chunk_len = strlen(chunk);
        // Check if we need to resize the result buffer
        if (current_size + chunk_len + 1 > buffer_size) {
            buffer_size = (current_size + chunk_len + 1) * 2;
            char* new_result = (char*)realloc(result, buffer_size);
            if (!new_result) {
                free(result);
                pclose(pipe);
                return NULL; // Reallocation failed
            }
            result = new_result;
        }
        strcat(result, chunk);
        current_size += chunk_len;
    }

    pclose(pipe);
    return result;
}

void run_test_case(const char* name, char* command) {
    printf("--- Test Case: %s ---\n", name);
    printf("Executing: %s\n", command);
    char* output = executeCommand(command);
    if (output) {
        printf("%s", output);
        free(output);
    } else {
        printf("Failed to execute command or allocate memory.\n");
    }
    printf("--------------------------\n\n");
}

int main() {
    // Test Case 1
    char* command1 = customPrint("%s %s", "ls", "-l");
    if (command1) {
        run_test_case("ls -l", command1);
        free(command1);
    }
    
    // Test Case 2
    char* command2 = customPrint("%s %s %s", "ls", "-a", "/tmp");
    if (command2) {
        run_test_case("ls -a /tmp", command2);
        free(command2);
    }
    
    // Test Case 3
    // Note: /var may not be readable by the current user
    char* command3 = customPrint("%s %s %s", "ls", "-lh", "/var");
    if (command3) {
        run_test_case("ls -lh /var", command3);
        free(command3);
    }
    
    // Test Case 4
    char* command4 = customPrint("%s %s", "ls", "/");
    if (command4) {
        run_test_case("ls /", command4);
        free(command4);
    }
    
    // Test Case 5
    char* command5 = customPrint("%s %s", "ls", "--version");
    if (command5) {
        run_test_case("ls --version", command5);
        free(command5);
    }

    return 0;
}