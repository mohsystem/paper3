#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Allowed characters in arguments. A whitelist approach is safer.
const char* ALLOWED_CHARS = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_./*?";

/**
 * @brief Validates an argument string to ensure it contains only allowed characters.
 * 
 * @param arg The argument string to validate.
 * @return 1 if the argument is safe, 0 otherwise.
 */
int is_valid_argument(const char* arg) {
    if (arg == NULL) {
        return 1; // Treat NULL as a skippable, valid case
    }
    // SECURITY: strspn returns the length of the initial segment of arg which consists
    // entirely of characters from ALLOWED_CHARS. If this length matches the
    // total length of arg, the argument is valid. This prevents command injection.
    return strspn(arg, ALLOWED_CHARS) == strlen(arg);
}


/**
 * @brief Executes the 'ls' command with the given arguments.
 * 
 * This function validates each argument to prevent command injection before
 * dynamically constructing the command string and passing it to system().
 * 
 * Note: Using non-shell-based APIs (like fork/exec on POSIX) is generally safer.
 * This implementation uses system() to match the prompt's requirements.
 * 
 * @param num_args The number of arguments in the args array.
 * @param args An array of strings representing the arguments for 'ls'.
 */
void executeLsCommand(int num_args, const char* args[]) {
    // Step 1: Validate all arguments and calculate required buffer size
    size_t total_len = 3; // "ls\0"
    for (int i = 0; i < num_args; ++i) {
        if (args[i] == NULL || args[i][0] == '\0') {
            continue;
        }
        if (!is_valid_argument(args[i])) {
            fprintf(stderr, "Error: Invalid or potentially malicious argument detected: %s\n", args[i]);
            return;
        }
        // length of arg + space
        total_len += strlen(args[i]) + 1;
    }
    
    // Step 2: Allocate memory for the command string
    char* command = (char*)malloc(total_len);
    if (command == NULL) {
        perror("Failed to allocate memory");
        return;
    }

    // Step 3: Build the command string safely.
    // The "print function" (strcpy, strcat) is used to create the string.
    strcpy(command, "ls");

    for (int i = 0; i < num_args; ++i) {
        if (args[i] == NULL || args[i][0] == '\0') {
            continue;
        }
        strcat(command, " ");
        strcat(command, args[i]);
    }
    
    printf("Executing command: %s\n", command);

    // Step 4: Execute the command using the system function
    int return_code = system(command);

    printf("system() returned: %d\n", return_code);

    // Step 5: Clean up
    free(command);
    command = NULL;
}

int main() {
    printf("--- Test Case 1: ls current directory ---\n");
    executeLsCommand(0, NULL);

    printf("\n--- Test Case 2: ls -l -a ---\n");
    const char* args2[] = {"-l", "-a"};
    executeLsCommand(2, args2);

    printf("\n--- Test Case 3: ls specific path ---\n");
    const char* args3[] = {"."};
    executeLsCommand(1, args3);

    printf("\n--- Test Case 4: ls non-existent file (shows error stream) ---\n");
    const char* args4[] = {"/non_existent_dir_12345"};
    executeLsCommand(1, args4);
    
    printf("\n--- Test Case 5: Attempted command injection (should be rejected) ---\n");
    const char* args5[] = {"-l;id"};
    executeLsCommand(1, args5);
    
    return 0;
}