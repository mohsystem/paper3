#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

/**
 * Runs the 'id <username>' command securely and returns its exit status.
 *
 * @param username The username to look up.
 * @return The exit status of the 'id' command. 0 for success, non-zero for failure.
 *         Returns -1 on a fork/exec error.
 */
int run_id_command(const char* username) {
    if (username == NULL || username[0] == '\0') {
        fprintf(stderr, "Error: Username cannot be null or empty.\n");
        return -1;
    }

    pid_t pid = fork();

    if (pid == -1) {
        // Fork failed
        perror("fork failed");
        return -1;
    } else if (pid == 0) {
        // Child process
        // Prepare arguments for execvp. It does not invoke a shell,
        // which prevents command injection vulnerabilities.
        char* argv[] = {"id", (char*)username, NULL};
        
        // Replace the child process with the 'id' command
        execvp("id", argv);
        
        // If execvp returns, an error occurred
        perror("execvp failed");
        _exit(127); // Use _exit to prevent flushing parent's stdio buffers
    } else {
        // Parent process
        int status;
        // Wait for the child process to terminate
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid failed");
            return -1;
        }

        // Check if the child terminated normally
        if (WIFEXITED(status)) {
            // Return the child's exit status
            return WEXITSTATUS(status);
        } else {
            // Child terminated abnormally
            return -1;
        }
    }
}

int main(int argc, char* argv[]) {
    // 5 test cases
    const char* test_usernames[] = {
        "root",                 // A user that usually exists
        "nobody",               // Another user that often exists
        "nonexistentuser12345", // A user that should not exist
        "; whoami",             // A potential command injection attempt
        "test user"             // A username with a space
    };
    int num_tests = sizeof(test_usernames) / sizeof(test_usernames[0]);

    printf("--- Running 5 Test Cases ---\n");
    for (int i = 0; i < num_tests; ++i) {
        printf("Testing username: '%s'\n", test_usernames[i]);
        int exit_status = run_id_command(test_usernames[i]);
        printf("Exit Status: %d\n\n", exit_status);
    }
    
    // Additionally, handle command line argument as per prompt description
    if (argc > 1) {
        printf("--- Running with command line argument ---\n");
        const char* user_from_arg = argv[1];
        printf("Testing username: '%s'\n", user_from_arg);
        int exit_status = run_id_command(user_from_arg);
        printf("Exit Status: %d\n", exit_status);
    }

    return 0;
}