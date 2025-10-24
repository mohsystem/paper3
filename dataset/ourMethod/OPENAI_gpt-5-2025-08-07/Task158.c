#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

static int is_valid_input(const char* s) {
    if (s == NULL) return 0;
    size_t n = strlen(s);
    if (n < 1 || n > 64) return 0;
    for (size_t i = 0; i < n; ++i) {
        unsigned char ch = (unsigned char)s[i];
        int ok = (ch >= 'A' && ch <= 'Z') ||
                 (ch >= 'a' && ch <= 'z') ||
                 (ch >= '0' && ch <= '9') ||
                 ch == ' ' || ch == '_' || ch == '-' || ch == ':' || ch == '.';
        if (!ok) return 0;
    }
    return 1;
}

static char* make_error(const char* msg) {
    size_t n = strlen(msg);
    char* out = (char*)malloc(n + 1);
    if (!out) return NULL;
    memcpy(out, msg, n + 1);
    return out;
}

static const char* find_printf_path(void) {
    if (access("/usr/bin/printf", X_OK) == 0) return "/usr/bin/printf";
    if (access("/bin/printf", X_OK) == 0) return "/bin/printf";
    return NULL;
}

char* run_safe_echo(const char* input) {
    if (!is_valid_input(input)) {
        return make_error("ERROR: Invalid input. Allowed: 1-64 chars [A-Za-z0-9 _:-.]");
    }

    const char* printf_path = find_printf_path();
    if (!printf_path) {
        return make_error("ERROR: printf not found on this system.");
    }

    int pipefd[2];
    if (pipe(pipefd) != 0) {
        return make_error("ERROR: pipe failed.");
    }

    pid_t pid = fork();
    if (pid < 0) {
        close(pipefd[0]);
        close(pipefd[1]);
        return make_error("ERROR: fork failed.");
    }

    if (pid == 0) {
        // Child
        if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
            _exit(127);
        }
        close(pipefd[0]);
        close(pipefd[1]);
        char* const argv[] = { "printf", "%s\n", (char*)input, NULL };
        execv(printf_path, argv);
        _exit(127);
    }

    // Parent
    close(pipefd[1]);

    size_t cap = 256;
    size_t len = 0;
    char* out = (char*)malloc(cap);
    if (!out) {
        close(pipefd[0]);
        kill(pid, SIGKILL);
        waitpid(pid, NULL, 0);
        return make_error("ERROR: memory allocation failed.");
    }

    int status = 0;
    int finished = 0;

    for (int i = 0; i < 200; ++i) { // ~2 seconds total waiting
        pid_t w = waitpid(pid, &status, WNOHANG);
        if (w == pid) {
            finished = 1;
            // Read remaining data
            char buf[1024];
            ssize_t r;
            while ((r = read(pipefd[0], buf, sizeof(buf))) > 0) {
                if (len + (size_t)r + 1 > cap) {
                    size_t ncap = (len + (size_t)r + 1) * 2;
                    char* tmp = (char*)realloc(out, ncap);
                    if (!tmp) {
                        close(pipefd[0]);
                        free(out);
                        return make_error("ERROR: memory allocation failed.");
                    }
                    out = tmp;
                    cap = ncap;
                }
                memcpy(out + len, buf, (size_t)r);
                len += (size_t)r;
                out[len] = '\0';
            }
            break;
        } else if (w == 0) {
            // Drain any available data
            char buf[1024];
            ssize_t r = read(pipefd[0], buf, sizeof(buf));
            if (r > 0) {
                if (len + (size_t)r + 1 > cap) {
                    size_t ncap = (len + (size_t)r + 1) * 2;
                    char* tmp = (char*)realloc(out, ncap);
                    if (!tmp) {
                        close(pipefd[0]);
                        free(out);
                        return make_error("ERROR: memory allocation failed.");
                    }
                    out = tmp;
                    cap = ncap;
                }
                memcpy(out + len, buf, (size_t)r);
                len += (size_t)r;
                out[len] = '\0';
            }
            usleep(10000); // 10ms
        } else {
            close(pipefd[0]);
            free(out);
            return make_error("ERROR: waitpid failed.");
        }
    }

    if (!finished) {
        kill(pid, SIGKILL);
        waitpid(pid, NULL, 0);
        close(pipefd[0]);
        free(out);
        return make_error("ERROR: Command timed out.");
    }

    close(pipefd[0]);
    if (len + 1 > cap) {
        char* tmp = (char*)realloc(out, len + 1);
        if (!tmp) {
            free(out);
            return make_error("ERROR: memory allocation failed.");
        }
        out = tmp;
    }
    out[len] = '\0';
    return out;
}

int main(void) {
    const char* tests[5] = {
        "HelloWorld",
        "Safe_123",
        "alpha-beta",
        "COLON:TEST",
        "Numbers 0123456789"
    };
    for (int i = 0; i < 5; ++i) {
        char* out = run_safe_echo(tests[i]);
        if (!out) {
            printf("Input: %s\nOutput: ERROR: unknown\n-----\n", tests[i]);
            continue;
        }
        printf("Input: %s\n", tests[i]);
        printf("Output: %s", out);
        if (out[0] && out[strlen(out) - 1] != '\n') printf("\n");
        printf("-----\n");
        free(out);
    }
    return 0;
}