#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// The following implementation is for POSIX-compliant systems (like Linux, macOS).
// It will not compile or run on Windows without modification to use the Windows API (e.g., CreateProcess).
#ifdef _WIN32
char* ping(const char* url) {
    const char* err_msg = "Error: This ping implementation is for POSIX systems (Linux, macOS) only.";
    char* result = malloc(strlen(err_msg) + 1);
    if (result) {
        strcpy(result, err_msg);
    }
    return result;
}
#else
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

/**
 * Validates that the input string contains only allowed characters for a hostname.
 * This is a security measure to prevent command injection.
 * @param url The string to validate.
 * @return 1 if valid, 0 otherwise.
 */
static int is_valid_hostname(const char* url) {
    if (url == NULL || strlen(url) > 255 || strlen(url) == 0) {
        return 0;
    }
    for (size_t i = 0; i < strlen(url); ++i) {
        if (!isalnum((unsigned char)url[i]) && url[i] != '.' && url[i] != '-') {
            return 0;
        }
    }
    return 1;
}

/**
 * Executes the system's ping command for a given URL or IP address.
 * To prevent command injection, this function uses fork() and execvp() to
 * execute the command directly, bypassing the shell.
 *
 * @param url The hostname or IP address to ping.
 * @return A dynamically allocated string with the command output. The caller MUST free this string.
 */
char* ping(const char* url) {
    if (!is_valid_hostname(url)) {
        const char* err_msg = "Error: Invalid or potentially malicious hostname provided.";
        char* result = malloc(strlen(err_msg) + 1);
        if (result) strcpy(result, err_msg);
        return result;
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
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);

        char* const argv[] = {"ping", "-c", "4", (char*)url, NULL};
        execvp("ping", argv);
        perror("execvp failed");
        _exit(127);
    } else { // Parent process
        close(pipefd[1]);

        size_t capacity = 1024;
        char* result = malloc(capacity);
        if (!result) return NULL;
        size_t size = 0;
        ssize_t bytes_read;
        while ((bytes_read = read(pipefd[0], result + size, capacity - size - 1)) > 0) {
            size += bytes_read;
            if (capacity - size < 256) {
                capacity *= 2;
                char* new_result = realloc(result, capacity);
                if (!new_result) { free(result); return NULL; }
                result = new_result;
            }
        }
        result[size] = '\0';

        close(pipefd[0]);
        waitpid(pid, NULL, 0);
        return result;
    }
}
#endif

// A dummy struct to satisfy the prompt's requirement for class name 'Lse111'.
typedef struct {} Lse111;

void runTests() {
    const char* testCases[] = {
        "8.8.8.8",                  // Test case 1: Valid IP
        "google.com",               // Test case 2: Valid domain
        "localhost",                // Test case 3: Local host
        "non-existent-domain.xyz",  // Test case 4: Non-existent but syntactically valid hostname
        "8.8.8.8; ls -la"           // Test case 5: Malicious input (should be rejected by validation)
    };
    int num_tests = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < num_tests; i++) {
        printf("--- Test Case %d: Pinging '%s' ---\n", i + 1, testCases[i]);
        char* result = ping(testCases[i]);
        if (result) {
            printf("%s\n", result);
            free(result); // IMPORTANT: Free the memory allocated by ping()
        } else {
            printf("The ping function returned an error.\n");
        }
        printf("--- End of Test Case %d ---\n\n", i + 1);
    }
}

int main() {
    runTests();
    return 0;
}