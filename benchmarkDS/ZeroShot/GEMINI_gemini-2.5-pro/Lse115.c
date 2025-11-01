#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdarg.h>

/**
 * Note: The prompt describes a pattern where a single formatted string is executed as a command.
 * This is a classic command injection vulnerability. A secure implementation, as provided below,
 * avoids this by never executing a command through a shell (like with system()). Instead, the command ("ls")
 * and its arguments are passed as a distinct array of strings to the execvp function.
 * This ensures that arguments, even if they contain shell metacharacters like ';' or '|',
 * are treated as literal strings and not as separate commands to be executed.
 */

/**
 * Creates a formatted string for use as a command argument.
 * This function corresponds to the "print function" described in the prompt,
 * but is used securely to format individual arguments, not the entire command string.
 * The caller is responsible for freeing the returned memory.
 *
 * @param format A C-style format string.
 * @param ...    Variable arguments for the format string.
 * @return A heap-allocated formatted string, or NULL on error.
 */
char* formatArgument(const char* format, ...) {
    va_list args1, args2;
    va_start(args1, format);
    va_copy(args2, args1);

    int size = vsnprintf(NULL, 0, format, args1);
    va_end(args1);

    if (size < 0) {
        va_end(args2);
        return NULL;
    }

    char* buffer = (char*)malloc(size + 1);
    if (!buffer) {
        va_end(args2);
        return NULL;
    }

    vsnprintf(buffer, size + 1, format, args2);
    va_end(args2);

    return buffer;
}


/**
 * Executes the 'ls' command with the given arguments in a secure way.
 * The caller is responsible for freeing the returned string.
 *
 * @param arg_count The number of arguments for the 'ls' command.
 * @param args An array of string arguments for the 'ls' command.
 * @return A heap-allocated string with the combined standard output and standard error, or NULL on error.
 */
char* executeLs(int arg_count, const char* args[]) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return NULL;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return NULL;
    }

    if (pid == 0) { // Child process
        close(pipefd[0]); // Close read end
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);

        const char** argv = (const char**)malloc(sizeof(const char*) * (arg_count + 2));
        if (!argv) _exit(127);
        argv[0] = "ls";
        for (int i = 0; i < arg_count; ++i) {
            argv[i + 1] = args[i];
        }
        argv[arg_count + 1] = NULL;
        
        execvp("ls", (char* const*)argv);
        perror("execvp");
        free(argv);
        _exit(127); // Exit if execvp fails
    } else { // Parent process
        close(pipefd[1]); // Close write end
        
        size_t capacity = 1024;
        size_t size = 0;
        char* output = (char*)malloc(capacity);
        if (!output) { close(pipefd[0]); return NULL; }
        
        char buffer[256];
        ssize_t bytes_read;
        while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
            if (size + bytes_read >= capacity) {
                capacity *= 2;
                char* new_output = (char*)realloc(output, capacity);
                if (!new_output) { free(output); close(pipefd[0]); return NULL; }
                output = new_output;
            }
            memcpy(output + size, buffer, bytes_read);
            size += bytes_read;
        }
        output[size] = '\0';
        close(pipefd[0]);

        int status;
        waitpid(pid, &status, 0);
        
        char status_str[50];
        snprintf(status_str, sizeof(status_str), "---\nExit Code: %d\n", WEXITSTATUS(status));
        size_t status_len = strlen(status_str);
        if (size + status_len >= capacity) {
             char* new_output = (char*)realloc(output, size + status_len + 1);
             if (!new_output) { free(output); return NULL; /* simplified error handling */ }
             output = new_output;
        }
        strcat(output, status_str);

        return output;
    }
}

void run_test(const char* test_name, int arg_count, const char* args[]) {
    printf("--- %s ---\n", test_name);
    char* result = executeLs(arg_count, args);
    if (result) {
        printf("%s\n", result);
        free(result);
    } else {
        printf("Failed to execute command.\n\n");
    }
}

int main() {
    run_test("Test Case 1: ls -l", 1, (const char*[]){"-l"});
    run_test("Test Case 2: ls -a /", 2, (const char*[]){"-a", "/"});
    run_test("Test Case 3: ls /nonexistent_dir", 1, (const char*[]){"/nonexistent_dir"});

    // Using the formatArgument function to build an argument
    char* formatted_arg = formatArgument("test_file_%d.txt", 123);
    if (formatted_arg) {
        char test_name[100];
        snprintf(test_name, sizeof(test_name), "Test Case 4: ls %s", formatted_arg);
        run_test(test_name, 1, (const char*[]){formatted_arg});
        free(formatted_arg);
    }

    // Security Test Case
    run_test("Test Case 5 (Security): ls '; id'", 1, (const char*[]){"; id"});
    
    return 0;
}