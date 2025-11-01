#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <errno.h>

static int is_valid_username(const char* u) {
    if (u == NULL) return 0;
    size_t len = strlen(u);
    if (len == 0 || len > 32) return 0;
    if (!(isalpha((unsigned char)u[0]) || u[0] == '_')) return 0;
    for (size_t i = 1; i < len; ++i) {
        unsigned char c = (unsigned char)u[i];
        if (!(isalpha(c) || isdigit(c) || c == '_' || c == '-')) return 0;
    }
    return 1;
}

// Caller must free the returned string. Returns empty allocated string on no match.
char* get_user_info(const char* username) {
    if (!is_valid_username(username)) {
        return NULL;
    }

    char pattern[64];
    // pattern is "^" + username + ":", ensure bounds
    size_t ulen = strlen(username);
    if (ulen + 2 + 1 >= sizeof(pattern)) { // '^' + ':' + '\0'
        return NULL;
    }
    pattern[0] = '^';
    memcpy(pattern + 1, username, ulen);
    pattern[1 + ulen] = ':';
    pattern[1 + ulen + 1] = '\0';

    int pipe1[2]; // cat -> grep
    int pipe2[2]; // grep -> parent
    if (pipe(pipe1) == -1) {
        return NULL;
    }
    if (pipe(pipe2) == -1) {
        close(pipe1[0]); close(pipe1[1]);
        return NULL;
    }

    pid_t pid1 = fork();
    if (pid1 < 0) {
        close(pipe1[0]); close(pipe1[1]);
        close(pipe2[0]); close(pipe2[1]);
        return NULL;
    }
    if (pid1 == 0) {
        // cat child
        if (dup2(pipe1[1], STDOUT_FILENO) == -1) _exit(127);
        close(pipe1[0]); close(pipe1[1]);
        close(pipe2[0]); close(pipe2[1]);
        execl("/bin/cat", "cat", "/etc/passwd", (char*)NULL);
        _exit(127);
    }

    pid_t pid2 = fork();
    if (pid2 < 0) {
        close(pipe1[0]); close(pipe1[1]);
        close(pipe2[0]); close(pipe2[1]);
        int st;
        waitpid(pid1, &st, 0);
        return NULL;
    }
    if (pid2 == 0) {
        // grep child
        if (dup2(pipe1[0], STDIN_FILENO) == -1) _exit(127);
        if (dup2(pipe2[1], STDOUT_FILENO) == -1) _exit(127);
        close(pipe1[0]); close(pipe1[1]);
        close(pipe2[0]); close(pipe2[1]);
        execl("/bin/grep", "grep", "-E", pattern, (char*)NULL);
        _exit(127);
    }

    // Parent
    close(pipe1[0]); close(pipe1[1]);
    close(pipe2[1]);

    const size_t MAX_READ = 2 * 1024 * 1024; // 2MB
    size_t cap = 1024;
    size_t len = 0;
    char* out = (char*)malloc(cap);
    if (!out) {
        close(pipe2[0]);
        int st;
        waitpid(pid1, &st, 0);
        waitpid(pid2, &st, 0);
        return NULL;
    }

    ssize_t r;
    char buf[8192];
    while ((r = read(pipe2[0], buf, sizeof(buf))) > 0) {
        if (len + (size_t)r + 1 > cap) {
            size_t newcap = cap;
            while (len + (size_t)r + 1 > newcap) {
                newcap *= 2;
                if (newcap > MAX_READ + 1) {
                    newcap = MAX_READ + 1;
                    break;
                }
            }
            if (newcap <= len + (size_t)r) {
                free(out);
                close(pipe2[0]);
                int st;
                waitpid(pid1, &st, 0);
                waitpid(pid2, &st, 0);
                return NULL;
            }
            char* tmp = (char*)realloc(out, newcap);
            if (!tmp) {
                free(out);
                close(pipe2[0]);
                int st;
                waitpid(pid1, &st, 0);
                waitpid(pid2, &st, 0);
                return NULL;
            }
            out = tmp;
            cap = newcap;
        }
        memcpy(out + len, buf, (size_t)r);
        len += (size_t)r;
        if (len > MAX_READ) {
            free(out);
            close(pipe2[0]);
            int st;
            waitpid(pid1, &st, 0);
            waitpid(pid2, &st, 0);
            return NULL;
        }
    }
    close(pipe2[0]);

    int st1, st2;
    waitpid(pid1, &st1, 0);
    waitpid(pid2, &st2, 0);

    if (out) {
        out[len] = '\0';
    }
    // Ensure non-NULL, even for empty
    if (!out) {
        out = (char*)calloc(1, 1);
    }
    return out;
}

int main(int argc, char** argv) {
    const char* tests[5];
    int test_count = 0;
    if (argc > 1) {
        tests[0] = argv[1];
        test_count = 1;
    } else {
        tests[0] = "root";
        tests[1] = "daemon";
        tests[2] = "bin";
        tests[3] = "nobody";
        tests[4] = "no_such_user_zzz";
        test_count = 5;
    }

    for (int i = 0; i < test_count; ++i) {
        printf("=== Query: %s ===\n", tests[i]);
        char* res = get_user_info(tests[i]);
        if (!res) {
            printf("(error)\n");
        } else {
            if (res[0] == '\0') {
                printf("(no match)\n");
            } else {
                fputs(res, stdout);
                if (res[strlen(res) - 1] != '\n') putchar('\n');
            }
            free(res);
        }
    }
    return 0;
}