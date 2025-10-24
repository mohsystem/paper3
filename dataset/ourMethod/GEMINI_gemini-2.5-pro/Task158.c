#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

// A whitelist of allowed directory names.
static const char* ALLOWED_DIRECTORIES[] = {"safe_dir_1", "safe_dir_2", ".", NULL};

// Function to check if a directory is in the whitelist
int is_allowed(const char* directory) {
    if (directory == NULL) {
        return 0;
    }
    for (int i = 0; ALLOWED_DIRECTORIES[i] != NULL; ++i) {
        if (strcmp(directory, ALLOWED_DIRECTORIES[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

/**
 * Executes a command with arguments safely, avoiding shell injection.
 * This implementation is for POSIX-compliant systems (e.g., Linux, macOS).
 *
 * @param directory The directory name to list. Must be in the allowed list.
 * @return A dynamically allocated string with the command's output, or NULL on error.
 *         The caller is responsible for freeing the returned string.
 */
char* listDirectoryContents(const char* directory) {
    if (!is_allowed(directory)) {
        fprintf(stderr, "Error: Invalid or disallowed directory specified.\n");
        return NULL;
    }

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return NULL;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return NULL;
    }

    if (pid == 0) { // Child process
        close(pipefd[0]); // Close unused read end
        dup2(pipefd[1], STDOUT_FILENO); // Redirect stdout to pipe
        dup2(pipefd[1], STDERR_FILENO); // Redirect stderr to pipe
        close(pipefd[1]);

        // Arguments for execvp. The user input is a single argument. This prevents shell interpretation.
        char* args[] = {"ls", "-l", (char*)directory, NULL};
        execvp(args[0], args);

        // execvp only returns on error
        perror("execvp");
        _exit(127);
    } else { // Parent process
        close(pipefd[1]); // Close unused write end

        size_t capacity = 1024;
        size_t size = 0;
        char* result = (char*)malloc(capacity);
        if (result == NULL) {
            perror("malloc");
            close(pipefd[0]);
            waitpid(pid, NULL, 0); // Clean up zombie process
            return NULL;
        }
        result[0] = '\0';

        ssize_t bytes_read;
        while ((bytes_read = read(pipefd[0], result + size, capacity - size - 1)) > 0) {
            size += bytes_read;
            if (size >= capacity - 1) {
                capacity *= 2;
                char* new_result = (char*)realloc(result, capacity);
                if (new_result == NULL) {
                    perror("realloc");
                    free(result);
                    close(pipefd[0]);
                    waitpid(pid, NULL, 0);
                    return NULL;
                }
                result = new_result;
            }
        }
        result[size] = '\0'; // Null-terminate the string
        close(pipefd[0]);

        int status;
        waitpid(pid, &status, 0);

        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Error: Command failed to execute properly. Output:\n%s\n", result);
            free(result);
            return NULL;
        }

        return result;
    }
}

int main() {
    // Note: Using system() here is only for test setup. The core function
    // avoids it for security.
    system("mkdir -p safe_dir_1");
    system("mkdir -p safe_dir_2");

    const char* testCases[] = {
        ".",
        "safe_dir_1",
        "safe_dir_2",
        "non_existent_dir",
        "safe_dir_1; whoami"
    };
    int num_tests = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < num_tests; ++i) {
        printf("--- Test Case %d: Input='%s' ---\n", i + 1, testCases[i]);
        char* result = listDirectoryContents(testCases[i]);
        if (result != NULL) {
            printf("%s\n", result);
            free(result);
        }
        printf("--- End Test Case %d ---\n\n", i + 1);
    }

    return 0;
}