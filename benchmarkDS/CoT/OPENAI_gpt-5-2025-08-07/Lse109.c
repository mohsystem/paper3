#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

static int validate_username(const char* username) {
    if (username == NULL) return 0;
    size_t len = strlen(username);
    if (len == 0 || len > 64) return 0;
    for (size_t i = 0; i < len; i++) {
        char c = username[i];
        if (!(isalnum((unsigned char)c) || c == '_' || c == '-' || c == '.')) {
            return 0;
        }
    }
    return 1;
}

// Returns a heap-allocated C string with output; caller must free() it.
char* get_user_info(const char* username) {
    if (!validate_username(username)) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }

    int pipe_cat_grep[2];
    int pipe_grep_parent[2];
    if (pipe(pipe_cat_grep) == -1) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    if (pipe(pipe_grep_parent) == -1) {
        close(pipe_cat_grep[0]); close(pipe_cat_grep[1]);
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }

    pid_t pid_cat = fork();
    if (pid_cat == -1) {
        close(pipe_cat_grep[0]); close(pipe_cat_grep[1]);
        close(pipe_grep_parent[0]); close(pipe_grep_parent[1]);
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }

    if (pid_cat == 0) {
        dup2(pipe_cat_grep[1], STDOUT_FILENO);
        close(pipe_cat_grep[0]);
        close(pipe_cat_grep[1]);
        close(pipe_grep_parent[0]);
        close(pipe_grep_parent[1]);
        execlp("cat", "cat", "/etc/passwd", (char*)NULL);
        _exit(127);
    }

    pid_t pid_grep = fork();
    if (pid_grep == -1) {
        close(pipe_cat_grep[0]); close(pipe_cat_grep[1]);
        close(pipe_grep_parent[0]); close(pipe_grep_parent[1]);
        waitpid(pid_cat, NULL, 0);
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }

    if (pid_grep == 0) {
        dup2(pipe_cat_grep[0], STDIN_FILENO);
        dup2(pipe_grep_parent[1], STDOUT_FILENO);
        close(pipe_cat_grep[0]);
        close(pipe_cat_grep[1]);
        close(pipe_grep_parent[0]);
        close(pipe_grep_parent[1]);
        execlp("grep", "grep", "-F", "--", username, (char*)NULL);
        _exit(127);
    }

    // Parent
    close(pipe_cat_grep[0]);
    close(pipe_cat_grep[1]);
    close(pipe_grep_parent[1]);

    size_t cap = 8192;
    size_t len = 0;
    char* out = (char*)malloc(cap);
    if (!out) {
        close(pipe_grep_parent[0]);
        waitpid(pid_cat, NULL, 0);
        waitpid(pid_grep, NULL, 0);
        return NULL;
    }

    char buf[8192];
    ssize_t r;
    while ((r = read(pipe_grep_parent[0], buf, sizeof(buf))) > 0) {
        if (len + (size_t)r + 1 > cap) {
            size_t new_cap = cap * 2;
            while (len + (size_t)r + 1 > new_cap) new_cap *= 2;
            char* new_out = (char*)realloc(out, new_cap);
            if (!new_out) {
                free(out);
                close(pipe_grep_parent[0]);
                waitpid(pid_cat, NULL, 0);
                waitpid(pid_grep, NULL, 0);
                return NULL;
            }
            out = new_out;
            cap = new_cap;
        }
        memcpy(out + len, buf, (size_t)r);
        len += (size_t)r;
    }
    close(pipe_grep_parent[0]);
    out[len] = '\0';

    waitpid(pid_cat, NULL, 0);
    waitpid(pid_grep, NULL, 0);

    return out;
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        char* res = get_user_info(argv[1]);
        if (res) {
            fputs(res, stdout);
            free(res);
        }
    } else {
        const char* tests[5] = {"root", "daemon", "nobody", "bin", "thisuserdoesnotexist123"};
        for (int i = 0; i < 5; i++) {
            char* res = get_user_info(tests[i]);
            if (res) {
                fputs(res, stdout);
                free(res);
            }
        }
    }
    return 0;
}