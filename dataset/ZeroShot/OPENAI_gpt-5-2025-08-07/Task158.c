#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <poll.h>
#include <fcntl.h>
#include <signal.h>

#define OUTPUT_LIMIT 8192
#define TIMEOUT_MS 5000

static int is_valid_host(const char* s) {
    if (s == NULL) return 0;
    size_t len = strlen(s);
    if (len < 1 || len > 253) return 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)s[i];
        if (!(isalnum(c) || c == '.' || c == '-')) return 0;
    }
    return 1;
}

static char* dup_str(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    memcpy(out, s, len + 1);
    return out;
}

char* run_ping(const char* user_input) {
    if (!is_valid_host(user_input)) {
        return dup_str("Invalid input");
    }

    const char* args[] = {"ping", "-c", "1", user_input, NULL};

    int pipefd[2];
    if (pipe(pipefd) != 0) {
        return dup_str("Execution failed");
    }

    // Set close-on-exec
    fcntl(pipefd[0], F_SETFD, fcntl(pipefd[0], F_GETFD) | FD_CLOEXEC);
    fcntl(pipefd[1], F_SETFD, fcntl(pipefd[1], F_GETFD) | FD_CLOEXEC);

    pid_t pid = fork();
    if (pid < 0) {
        close(pipefd[0]);
        close(pipefd[1]);
        return dup_str("Execution failed");
    } else if (pid == 0) {
        // Child
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        execvp(args[0], (char* const*)args);
        _exit(127);
    }

    // Parent
    close(pipefd[1]);
    int flags = fcntl(pipefd[0], F_GETFL, 0);
    if (flags >= 0) fcntl(pipefd[0], F_SETFL, flags | O_NONBLOCK);

    char* output = (char*)malloc(OUTPUT_LIMIT + 1);
    if (!output) {
        kill(pid, SIGKILL);
        waitpid(pid, NULL, 0);
        close(pipefd[0]);
        return dup_str("Execution failed");
    }
    size_t out_len = 0;
    output[0] = '\0';

    struct pollfd pfd;
    pfd.fd = pipefd[0];
    pfd.events = POLLIN | POLLHUP | POLLERR;

    int elapsed = 0;
    const int step = 100; // ms
    int pipe_open = 1;
    int child_exited = 0;

    while (elapsed < TIMEOUT_MS) {
        int ret = poll(&pfd, 1, step);
        if (ret > 0) {
            if (pfd.revents & POLLIN) {
                char buf[512];
                ssize_t r = read(pipefd[0], buf, sizeof(buf));
                if (r > 0) {
                    size_t remain = OUTPUT_LIMIT - out_len;
                    if (remain > 0) {
                        size_t to_copy = (size_t)r < remain ? (size_t)r : remain;
                        memcpy(output + out_len, buf, to_copy);
                        out_len += to_copy;
                        output[out_len] = '\0';
                    }
                } else if (r == 0) {
                    pipe_open = 0;
                }
            }
            if (pfd.revents & (POLLHUP | POLLERR)) {
                char buf[512];
                ssize_t r;
                while ((r = read(pipefd[0], buf, sizeof(buf))) > 0) {
                    size_t remain = OUTPUT_LIMIT - out_len;
                    if (remain == 0) break;
                    size_t to_copy = (size_t)r < remain ? (size_t)r : remain;
                    memcpy(output + out_len, buf, to_copy);
                    out_len += to_copy;
                    output[out_len] = '\0';
                }
                pipe_open = 0;
            }
        }
        int status = 0;
        pid_t w = waitpid(pid, &status, WNOHANG);
        if (w == pid) child_exited = 1;

        if (!pipe_open && child_exited) break;
        elapsed += step;
    }

    close(pipefd[0]);

    if (!( !pipe_open && child_exited )) {
        kill(pid, SIGKILL);
        waitpid(pid, NULL, 0);
        free(output);
        return dup_str("Timed out");
    }

    output[out_len] = '\0';
    return output;
}

int main(void) {
    const char* tests[5] = {
        "localhost",
        "127.0.0.1",
        "example.com",
        "bad;rm -rf /",
        "name with spaces"
    };
    for (int i = 0; i < 5; i++) {
        char* out = run_ping(tests[i]);
        printf("Input: %s\n", tests[i]);
        printf("Output (truncated):\n%s\n", out ? out : "(null)");
        printf("----\n");
        free(out);
    }
    return 0;
}