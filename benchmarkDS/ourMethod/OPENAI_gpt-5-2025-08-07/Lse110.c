#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>

static int is_ascii_string(const char *s) {
    if (!s) return 0;
    for (const unsigned char *p = (const unsigned char *)s; *p; ++p) {
        if ((*p) & 0x80) return 0;
    }
    return 1;
}

// Validate username: length 1..32, ASCII only, starts with [A-Za-z_],
// then [A-Za-z0-9_.-], optional trailing '$'
int is_valid_username(const char *u) {
    if (u == NULL) return 0;
    size_t len = strlen(u);
    if (len < 1 || len > 32) return 0;
    if (!is_ascii_string(u)) return 0;

    int has_trailing_dollar = (u[len - 1] == '$');
    size_t base_len = has_trailing_dollar ? len - 1 : len;
    if (base_len < 1) return 0;

    unsigned char first = (unsigned char)u[0];
    if (!(isalpha(first) || first == '_')) return 0;

    for (size_t i = 1; i < base_len; ++i) {
        unsigned char c = (unsigned char)u[i];
        if (!(isalnum(c) || c == '_' || c == '-' || c == '.')) return 0;
    }
    return 1;
}

int run_id_command(const char *username) {
    if (!is_valid_username(username)) {
        fprintf(stderr, "Invalid username.\n");
        return 2; // invalid input
    }

    pid_t pid = fork();
    if (pid < 0) {
        return 127; // fork failed
    } else if (pid == 0) {
        execlp("id", "id", username, (char *)NULL);
        _exit(127); // exec failed
    } else {
        int status = 0;
        if (waitpid(pid, &status, 0) < 0) {
            return 127;
        }
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            return 128 + WTERMSIG(status);
        } else {
            return 127;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc >= 2) {
        int rc = run_id_command(argv[1]);
        printf("Exit status: %d\n", rc);
        return 0;
    }
    // 5 test cases
    const char *tests[5] = {"root", "daemon", "nobody", "bin", "this_user_should_not_exist_zzz"};
    for (int i = 0; i < 5; ++i) {
        printf("Running: id %s\n", tests[i]);
        int rc = run_id_command(tests[i]);
        printf("Exit status: %d\n", rc);
    }
    return 0;
}