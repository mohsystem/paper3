#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <unistd.h>
#include <sys/wait.h>

// Note: This C code is POSIX-specific (Linux, macOS) due to its use of fork, execvp, and pipe.
// This is the secure way to run external commands without involving a shell,
// thus preventing command injection at the execution level.

// Regex for validating a hostname or IP address.
// This is the primary defense against command injection.
int isValidHostnameOrIp(const char* host) {
    regex_t regex;
    int reti;
    // Simplified regex allowing alphanumeric chars, dots, and hyphens.
    // This blocks shell metacharacters like ';', '|', '&', '`', '$', etc.
    const char* pattern = "^[a-zA-Z0-9\\.\\-]*$";

    reti = regcomp(&regex, pattern, REG_EXTENDED | REG_NOSUB);
    if (reti) {
        fprintf(stderr, "Could not compile regex\n");
        return 0;
    }

    reti = regexec(&regex, host, 0, NULL, 0);
    regfree(&regex);

    if (reti == 0) {
        return 1; // Match
    } else if (reti == REG_NOMATCH) {
        return 0; // No match
    } else {
        char msgbuf[100];
        regerror(reti, &regex, msgbuf, sizeof(msgbuf));
        fprintf(stderr, "Regex match failed: %s\n", msgbuf);
        return 0;
    }
}

/**
 * Executes the ping command for a given URL/hostname in a secure way.
 *
 * @param url The URL or IP address to ping.
 * @return A dynamically allocated string with the command output.
 *         The caller is responsible for freeing this memory.
 *         Returns NULL on failure.
 */
char* ping(const char* url) {
    // Rule #1: Input Validation
    if (url == NULL || !isValidHostnameOrIp(url) || strlen(url) > 253) {
        char* error_msg = (char*)malloc(128);
        if (error_msg) {
             snprintf(error_msg, 128, "Error: Invalid or potentially malicious hostname/IP provided.");
        }
        return error_msg;
    }

    int pipefd[2];
    pid_t pid;

    if (pipe(pipefd) == -1) {
        perror("pipe");
        return NULL;
    }

    pid = fork();
    if (pid == -1) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return NULL;
    }

    if (pid == 0) { // Child process
        close(pipefd[0]); // Close unused read end
        // Redirect stdout and stderr to the pipe
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);

        // Rule #2: Use safer alternatives. execvp avoids the shell.
        char* const argv[] = {"ping", "-c", "4", (char*)url, NULL};
        execvp("ping", argv);
        
        // execvp only returns on error
        perror("execvp");
        _exit(127);
    } else { // Parent process
        close(pipefd[1]); // Close unused write end

        char buffer[256];
        ssize_t count;
        size_t total_size = 1; // Start with 1 for null terminator
        char* output = (char*)malloc(total_size);
        if (output == NULL) {
            perror("malloc");
            close(pipefd[0]);
            wait(NULL);
            return NULL;
        }
        output[0] = '\0';

        // Rule #3: Safe buffer operations
        while ((count = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[count] = '\0';
            size_t new_size = total_size + count;
            char* new_output = (char*)realloc(output, new_size);
            if (new_output == NULL) {
                perror("realloc");
                free(output);
                close(pipefd[0]);
                wait(NULL);
                return NULL;
            }
            output = new_output;
            strncat(output, buffer, count);
            total_size = new_size;
        }

        close(pipefd[0]);
        wait(NULL); // Wait for child to finish

        return output;
    }
    return NULL; // Should not be reached
}


int main() {
    // The prompt's decorator concept points to a web server context, which is too complex
    // for a single C file example. We demonstrate the core function with test cases.
    const char* testUrls[] = {
        "8.8.8.8",
        "localhost",
        "example.com",
        "127.0.0.1",
        "invalid-hostname; ls -la" // Malicious input
    };
    int num_tests = sizeof(testUrls) / sizeof(testUrls[0]);

    printf("--- Running 5 Test Cases ---\n");
    for (int i = 0; i < num_tests; ++i) {
        printf("\n--- Test Case %d: pinging '%s' ---\n", i + 1, testUrls[i]);
        char* result = ping(testUrls[i]);
        if (result) {
            printf("%s", result);
            free(result); // Free the memory allocated by ping()
        } else {
            printf("Ping function failed to return output.\n");
        }
        printf("--- End Test Case %d ---\n", i + 1);
    }

    return 0;
}