#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

char* sprint(const char* fmt, ...) {
    if (!fmt) return NULL;
    va_list ap;
    va_start(ap, fmt);
    va_list ap2;
    va_copy(ap2, ap);
    int n = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    if (n < 0) {
        va_end(ap2);
        return NULL;
    }
    char* buf = (char*)malloc((size_t)n + 1);
    if (!buf) {
        va_end(ap2);
        return NULL;
    }
    vsnprintf(buf, (size_t)n + 1, fmt, ap2);
    va_end(ap2);
    return buf;
}

int is_safe_path(const char* s) {
    if (!s) return 0;
    size_t len = strlen(s);
    if (len == 0 || len > 4096) return 0;
    if (s[0] == '-') return 0;
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)s[i];
        if (!(isalnum(c) || c==' ' || c=='/' || c=='.' || c=='_' || c=='-')) {
            return 0;
        }
    }
    return 1;
}

char* execute_ls(const char* arg) {
    const char* path = (arg == NULL || arg[0] == '\0') ? "." : arg;
    if (!is_safe_path(path)) {
        char* msg = strdup("Invalid or unsafe path input.\n");
        return msg ? msg : NULL;
    }

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        char* msg = (char*)malloc(256);
        if (!msg) return NULL;
        snprintf(msg, 256, "pipe failed: %s\n", strerror(errno));
        return msg;
    }

    // Set close-on-exec
    fcntl(pipefd[0], F_SETFD, fcntl(pipefd[0], F_GETFD) | FD_CLOEXEC);
    fcntl(pipefd[1], F_SETFD, fcntl(pipefd[1], F_GETFD) | FD_CLOEXEC);

    pid_t pid = fork();
    if (pid < 0) {
        close(pipefd[0]); close(pipefd[1]);
        char* msg = (char*)malloc(256);
        if (!msg) return NULL;
        snprintf(msg, 256, "fork failed: %s\n", strerror(errno));
        return msg;
    }

    if (pid == 0) {
        // Child
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);

        char* argv[5];
        argv[0] = "ls";
        argv[1] = "--color=never";
        argv[2] = "--";
        argv[3] = (char*)path;
        argv[4] = NULL;

        execvp("ls", argv);
        _exit(127);
    }

    // Parent
    close(pipefd[1]);

    size_t cap = 8192;
    size_t len = 0;
    char* out = (char*)malloc(cap);
    if (!out) {
        close(pipefd[0]);
        // best effort to reap child
        int status;
        waitpid(pid, &status, 0);
        return NULL;
    }

    const size_t MAX_OUT = 10 * 1024 * 1024; // 10MB
    char buf[8192];
    ssize_t r;
    while ((r = read(pipefd[0], buf, sizeof(buf))) > 0) {
        if (len + (size_t)r + 1 > cap) {
            size_t newcap = cap * 2;
            if (newcap < len + (size_t)r + 1) newcap = len + (size_t)r + 1;
            if (newcap > MAX_OUT + 1024) newcap = MAX_OUT + 1024; // limit growth
            char* tmp = (char*)realloc(out, newcap);
            if (!tmp) {
                // Truncate and note
                const char* trunc = "\n[Output truncated]\n";
                size_t tlen = strlen(trunc);
                if (len + tlen + 1 <= cap) {
                    memcpy(out + len, trunc, tlen);
                    len += tlen;
                }
                break;
            }
            out = tmp;
            cap = newcap;
        }
        if (len + (size_t)r > MAX_OUT) {
            size_t to_copy = (MAX_OUT > len) ? (MAX_OUT - len) : 0;
            if (to_copy > 0) {
                memcpy(out + len, buf, to_copy);
                len += to_copy;
            }
            const char* trunc = "\n[Output truncated]\n";
            size_t tlen = strlen(trunc);
            if (len + tlen + 1 <= cap) {
                memcpy(out + len, trunc, tlen);
                len += tlen;
            }
            // Drain remaining
            while ((r = read(pipefd[0], buf, sizeof(buf))) > 0) { /* discard */ }
            break;
        } else {
            memcpy(out + len, buf, (size_t)r);
            len += (size_t)r;
        }
    }
    close(pipefd[0]);
    int status = 0;
    waitpid(pid, &status, 0);
    (void)status;

    if (len + 1 > cap) {
        char* tmp = (char*)realloc(out, len + 1);
        if (tmp) {
            out = tmp;
            cap = len + 1;
        }
    }
    out[len] = '\0';
    return out;
}

int main(void) {
    char* tests[5];
    tests[0] = sprint("%s", ".");
    tests[1] = sprint("%s", "/");
    tests[2] = sprint("%s", "..");
    tests[3] = sprint("./%s", "nonexistent_dir_hopefully");
    tests[4] = sprint("%s-%s", "src", "main");

    for (int i = 0; i < 5; ++i) {
        if (!tests[i]) tests[i] = strdup(".");
        printf("Test %d input: %s\n", i + 1, tests[i]);
        char* result = execute_ls(tests[i]);
        if (result) {
            fputs(result, stdout);
            free(result);
        } else {
            puts("Execution failed.\n");
        }
        puts("----");
        free(tests[i]);
    }
    return 0;
}