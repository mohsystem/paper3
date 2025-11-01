#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>

#define INVALID_INPUT_STATUS 2
#define TIMEOUT_STATUS 124

static int is_valid_username(const char* username) {
    if (username == NULL) return 0;
    size_t len = strlen(username);
    if (len == 0 || len > 128) return 0;

    // First char: [a-z_]
    char c = username[0];
    if (!( (c >= 'a' && c <= 'z') || c == '_' )) return 0;

    // Subsequent chars: [a-z0-9_-], allow single trailing $
    for (size_t i = 1; i < len; ++i) {
        char ch = username[i];
        if (ch == '$') {
            // Only allowed at last position
            if (i != len - 1) return 0;
            continue;
        }
        if (!((ch >= 'a' && ch <= 'z') ||
              (ch >= '0' && ch <= '9') ||
              ch == '_' || ch == '-')) {
            return 0;
        }
    }
    return 1;
}

int run_id(const char* username) {
    if (!is_valid_username(username)) {
        return INVALID_INPUT_STATUS;
    }

    pid_t pid = fork();
    if (pid < 0) {
        return 1;
    } else if (pid == 0) {
        // Child process
        char *const argv1[] = {"/usr/bin/id", (char*)username, NULL};
        execv("/usr/bin/id", argv1);
        if (errno == ENOENT) {
            char *const argv2[] = {"/bin/id", (char*)username, NULL};
            execv("/bin/id", argv2);
        }
        _exit(127);
    } else {
        // Parent, wait with timeout (5 seconds)
        int status = 0;
        const int timeout_ms = 5000;
        int waited = 0;
        while (waited < timeout_ms) {
            pid_t r = waitpid(pid, &status, WNOHANG);
            if (r == pid) {
                if (WIFEXITED(status)) return WEXITSTATUS(status);
                if (WIFSIGNALED(status)) return 128 + WTERMSIG(status);
                return 1;
            } else if (r == 0) {
                struct timespec ts;
                ts.tv_sec = 0;
                ts.tv_nsec = 10 * 1000000L; // 10ms
                nanosleep(&ts, NULL);
                waited += 10;
                continue;
            } else {
                // waitpid error
                return 1;
            }
        }
        // Timeout: kill child
        kill(pid, SIGKILL);
        waitpid(pid, NULL, 0);
        return TIMEOUT_STATUS;
    }
}

int main(int argc, char** argv) {
    if (argc >= 2) {
        const char* user = argv[1];
        int status = run_id(user);
        printf("Exit status for username '%s': %d\n", user, status);
    } else {
        const char* tests[5] = {"root", "nobody", "daemon", "syslog", "nonexistentuser12345"};
        for (int i = 0; i < 5; ++i) {
            int status = run_id(tests[i]);
            printf("Exit status for username '%s': %d\n", tests[i], status);
        }
    }
    return 0;
}