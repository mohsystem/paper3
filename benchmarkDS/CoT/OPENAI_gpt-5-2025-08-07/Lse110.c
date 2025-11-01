/* Chain-of-Through process in code generation:
 * 1) Problem understanding: Implement function taking username, running '/usr/bin/id -- <username>', returning exit status; print it.
 * 2) Security requirements: Avoid shell, use absolute path, prevent option injection with '--', handle errors.
 * 3) Secure coding generation: Use fork/execv, waitpid for exit status.
 * 4) Code review: No shell use, absolute path, '--' used, proper status decoding.
 * 5) Secure code output: Final code with robust handling and 5 test cases.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

int run_id(const char* username) {
    if (username == NULL || username[0] == '\0') {
        return 2; // invalid input
    }
    pid_t pid = fork();
    if (pid < 0) {
        return 1; // fork failed
    }
    if (pid == 0) {
        // Child process
        execl("/usr/bin/id", "id", "--", username, (char*)NULL);
        _exit(127); // exec failed
    }
    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
        return 1;
    }
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        return 128 + WTERMSIG(status);
    }
    return 1;
}

int main(int argc, char* argv[]) {
    if (argc >= 2) {
        int status = run_id(argv[1]);
        printf("Exit status: %d\n", status);
        return 0;
    }
    // 5 test cases
    const char* tests[5] = {
        "root",
        "nobody",
        "daemon",
        "www-data",
        "user_does_not_exist_123456"
    };
    for (int i = 0; i < 5; ++i) {
        int status = run_id(tests[i]);
        printf("Username: %s -> Exit status: %d\n", tests[i], status);
    }
    return 0;
}