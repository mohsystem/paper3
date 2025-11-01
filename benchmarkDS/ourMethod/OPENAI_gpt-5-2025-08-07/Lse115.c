#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <limits.h>

static int is_safe_path(const char* s) {
    if (s == NULL) return 0;
    size_t len = strlen(s);
    if (len > 255) return 0;
    // No NUL inside string apart from terminator - ensured by strlen
    if (strstr(s, "..") != NULL) return 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)s[i];
        if (!(isalnum(c) || c == '.' || c == '_' || c == '-' || c == '/')) {
            return 0;
        }
    }
    return 1;
}

static char* build_string(const char* fmt, ...) {
    if (fmt == NULL) return NULL;
    va_list ap;
    va_start(ap, fmt);
    va_list ap2;
    va_copy(ap2, ap);
    int needed = vsnprintf(NULL, 0, fmt, ap2);
    va_end(ap2);
    if (needed < 0 || needed > 255) {
        va_end(ap);
        return NULL;
    }
    size_t size = (size_t)needed + 1U;
    char* buf = (char*)malloc(size);
    if (!buf) {
        va_end(ap);
        return NULL;
    }
    int written = vsnprintf(buf, size, fmt, ap);
    va_end(ap);
    if (written < 0 || written > needed) {
        free(buf);
        return NULL;
    }
    if (!is_safe_path(buf)) {
        free(buf);
        return NULL;
    }
    return buf;
}

static char* execute_ls(const char* input) {
    const char* path = (input != NULL && input[0] != '\0') ? input : ".";
    if (!is_safe_path(path)) {
        const char* err = "error: invalid input\n";
        size_t len = strlen(err) + 1U;
        char* msg = (char*)malloc(len);
        if (msg) memcpy(msg, err, len);
        return msg;
    }

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        const char* err = "error: pipe\n";
        size_t len = strlen(err) + 1U;
        char* msg = (char*)malloc(len);
        if (msg) memcpy(msg, err, len);
        return msg;
    }

    pid_t pid = fork();
    if (pid == -1) {
        close(pipefd[0]);
        close(pipefd[1]);
        const char* err = "error: fork\n";
        size_t len = strlen(err) + 1U;
        char* msg = (char*)malloc(len);
        if (msg) memcpy(msg, err, len);
        return msg;
    }

    if (pid == 0) {
        // Child: redirect stdout and stderr to pipe
        if (dup2(pipefd[1], STDOUT_FILENO) == -1) _exit(127);
        if (dup2(pipefd[1], STDERR_FILENO) == -1) _exit(127);
        close(pipefd[0]);
        close(pipefd[1]);
        const char* argv_child[] = {"ls", "--", path, NULL};
        execvp("ls", (char* const*)argv_child);
        _exit(127);
    }

    // Parent
    close(pipefd[1]);
    size_t cap = 4096U;
    size_t used = 0U;
    char* out = (char*)malloc(cap);
    if (!out) {
        close(pipefd[0]);
        (void)waitpid(pid, NULL, 0);
        return NULL;
    }

    for (;;) {
        char buf[4096];
        ssize_t n = read(pipefd[0], buf, sizeof(buf));
        if (n < 0) {
            // Read error
            free(out);
            close(pipefd[0]);
            (void)waitpid(pid, NULL, 0);
            const char* err = "error: read\n";
            size_t len = strlen(err) + 1U;
            char* msg = (char*)malloc(len);
            if (msg) memcpy(msg, err, len);
            return msg;
        } else if (n == 0) {
            break;
        } else {
            // Ensure capacity for n bytes plus terminator
            if (used + (size_t)n + 1U < used) {
                // overflow
                free(out);
                close(pipefd[0]);
                (void)waitpid(pid, NULL, 0);
                return NULL;
            }
            size_t required = used + (size_t)n + 1U;
            if (required > cap) {
                size_t newcap = cap;
                while (newcap < required) {
                    if (newcap > (SIZE_MAX / 2U)) {
                        newcap = required;
                        break;
                    }
                    newcap *= 2U;
                }
                char* tmp = (char*)realloc(out, newcap);
                if (!tmp) {
                    free(out);
                    close(pipefd[0]);
                    (void)waitpid(pid, NULL, 0);
                    return NULL;
                }
                out = tmp;
                cap = newcap;
            }
            memcpy(out + used, buf, (size_t)n);
            used += (size_t)n;
            out[used] = '\0';
        }
    }
    close(pipefd[0]);
    (void)waitpid(pid, NULL, 0);
    // Ensure null termination
    if (used >= cap) {
        char* tmp = (char*)realloc(out, used + 1U);
        if (!tmp) {
            free(out);
            return NULL;
        }
        out = tmp;
        cap = used + 1U;
    }
    out[used] = '\0';
    return out;
}

int main(void) {
    char* tests[5];
    tests[0] = build_string("%s", ".");               // current directory
    tests[1] = build_string("%s", "/");               // root
    tests[2] = build_string("nonexistent_%s", "dir"); // likely nonexistent
    tests[3] = build_string("%s/%s", "tmp", "test");  // nested path
    tests[4] = build_string("%s%s", ".", "");         // another current dir

    for (int i = 0; i < 5; i++) {
        printf("=== Test %d ===\n", i + 1);
        const char* in = tests[i];
        if (in == NULL) in = "";
        char* out = execute_ls(in);
        if (out) {
            fputs(out, stdout);
            free(out);
        } else {
            fputs("error: execution failed\n", stdout);
        }
        if (tests[i]) {
            free(tests[i]);
        }
    }
    return 0;
}