#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/**
 * Validates an argument string by checking for forbidden shell metacharacters.
 * @param arg The string to validate.
 * @return true if the string is safe, false otherwise.
 */
bool is_argument_safe(const char* arg) {
    // Rule #1: Validate input against a blacklist of shell metacharacters.
    if (arg == NULL) {
        return true; // NULL is safe (treated as empty).
    }
    const char* forbidden_chars = ";&|`'\"$*<>(){}\\";
    // strpbrk returns a pointer to the first character in arg that matches
    // any character in forbidden_chars, or NULL if no such character is found.
    return strpbrk(arg, forbidden_chars) == NULL;
}

/**
 * Constructs and executes an 'ls' command using the system() call.
 *
 * @param path The directory path to list.
 * @param flags The flags to pass to the 'ls' command (e.g., "-l").
 */
void executeLsCommand(const char* path, const char* flags) {
    if (!is_argument_safe(path) || !is_argument_safe(flags)) {
        fprintf(stderr, "Error: Potentially unsafe characters detected in input. Aborting.\n");
        return;
    }

    // Rule #3: Ensure buffer operations adhere to boundaries.
    // Safely calculate required buffer size.
    size_t flags_len = (flags == NULL) ? 0 : strlen(flags);
    size_t path_len = (path == NULL) ? 0 : strlen(path);
    
    // Check for potential integer overflow before calculating total size.
    if (flags_len > 1024 || path_len > 4096) {
        fprintf(stderr, "Error: Input argument too long.\n");
        return;
    }

    // Size for "ls ", flags, " ", path, and null terminator
    size_t required_size = 3 + flags_len + 1 + path_len + 1;
    char* command = (char*)malloc(required_size);
    if (command == NULL) {
        perror("Error: Memory allocation failed");
        return;
    }

    // The "print function" (snprintf) is used to create the string for the system function.
    // It is safe against buffer overflows.
    int written_chars = snprintf(command, required_size, "ls %s %s", 
                                 (flags ? flags : ""), 
                                 (path ? path : ""));

    if (written_chars < 0 || (size_t)written_chars >= required_size) {
        fprintf(stderr, "Error: snprintf failed to format command.\n");
        free(command);
        return;
    }

    printf("Executing command: %s\n", command);
    printf("---\n");

    // Rule #2: Use potentially dangerous functions with caution.
    // system() invokes a shell, which is a security risk if the command string
    // is not constructed from validated and sanitized inputs.
    int return_code = system(command);

    printf("---\n");
    // The return code format is implementation-defined.
    printf("Command finished with exit code: %d\n", return_code);

    free(command); // Free the allocated memory.
}

int main() {
    printf("--- Test Case 1: List current directory ---\n");
    executeLsCommand(".", "");

    printf("\n--- Test Case 2: List /tmp with long format ---\n");
    executeLsCommand("/tmp", "-l");

    printf("\n--- Test Case 3: Non-existent directory ---\n");
    executeLsCommand("/non_existent_dir_12345", "-a");

    printf("\n--- Test Case 4: Invalid path (command injection attempt) ---\n");
    executeLsCommand(".; ls", "-l");

    printf("\n--- Test Case 5: Invalid flag (command injection attempt) ---\n");
    executeLsCommand(".", "-l; echo vulnerable");

    return 0;
}