#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

static int is_valid_username(const char* username) {
    if (username == NULL) return 0;
    size_t len = strlen(username);
    if (len == 0 || len > 32) return 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)username[i];
        if (!(isalnum(c) || c == '_' || c == '-' || c == '.')) {
            return 0;
        }
    }
    return 1;
}

static void set_cloexec(int fd) {
    int flags = fcntl(fd, F_GETFD);
    if (flags != -1) fcntl(fd, F_SETFD, flags | FD_CLOEXEC);
}

// Execute two-command pipeline cmd1 | cmd2
// Each cmd is a NULL-terminated array of const char*
char* exec_pipeline(const char* const cmd1[], const char* const cmd2[]) {
    int pipe_cmd[2];
    int pipe_out[2];
    if (pipe(pipe_cmd) == -1) return NULL;
    if (pipe(pipe_out) == -1) { close(pipe_cmd[0]); close(pipe_cmd[1]); return NULL; }

    set_cloexec(pipe_cmd[0]); set_cloexec(pipe_cmd[1]);
    set_cloexec(pipe_out[0]); set_cloexec(pipe_out[1]);

    pid_t c1 = fork();
    if (c1 == -1) {
        close(pipe_cmd[0]); close(pipe_cmd[1]); close(pipe_out[0]); close(pipe_out[1]);
        return NULL;
    }
    if (c1 == 0) {
        // child 1: cmd1
        dup2(pipe_cmd[1], STDOUT_FILENO);
        close(pipe_cmd[0]); close(pipe_cmd[1]);
        close(pipe_out[0]); close(pipe_out[1]);
        execv(cmd1[0], (char* const*)cmd1);
        _exit(127);
    }

    pid_t c2 = fork();
    if (c2 == -1) {
        close(pipe_cmd[0]); close(pipe_cmd[1]); close(pipe_out[0]); close(pipe_out[1]);
        waitpid(c1, NULL, 0);
        return NULL;
    }
    if (c2 == 0) {
        // child 2: cmd2
        dup2(pipe_cmd[0], STDIN_FILENO);
        dup2(pipe_out[1], STDOUT_FILENO);
        close(pipe_cmd[0]); close(pipe_cmd[1]);
        close(pipe_out[0]); close(pipe_out[1]);
        execv(cmd2[0], (char* const*)cmd2);
        _exit(127);
    }

    // Parent
    close(pipe_cmd[0]); close(pipe_cmd[1]);
    close(pipe_out[1]);

    size_t cap = 4096, len = 0;
    char* out = (char*)malloc(cap);
    if (!out) { close(pipe_out[0]); return NULL; }

    ssize_t r;
    char buf[8192];
    while ((r = read(pipe_out[0], buf, sizeof(buf))) > 0) {
        if (len + (size_t)r + 1 > cap) {
            size_t ncap = (cap * 2 > len + (size_t)r + 1) ? cap * 2 : (len + (size_t)r + 1);
            char* tmp = (char*)realloc(out, ncap);
            if (!tmp) { free(out); close(pipe_out[0]); return NULL; }
            out = tmp; cap = ncap;
        }
        memcpy(out + len, buf, (size_t)r);
        len += (size_t)r;
    }
    close(pipe_out[0]);
    out[len] = '\0';

    int status;
    waitpid(c1, &status, 0);
    waitpid(c2, &status, 0);

    return out;
}

char* get_user_info(const char* username) {
    if (!is_valid_username(username)) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    const char* cmd1[] = { "/bin/cat", "/etc/passwd", NULL };
    const char* cmd2[] = { "/bin/grep", "-F", username, NULL };
    char* result = exec_pipeline(cmd1, cmd2);
    if (!result) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    return result;
}

int main(int argc, char* argv[]) {
    const char* tests[5];
    tests[0] = (argc > 1) ? argv[1] : "root";
    tests[1] = "daemon";
    tests[2] = "bin";
    tests[3] = "nosuchuser";
    tests[4] = "invalid;rm";

    for (int i = 0; i < 5; i++) {
        const char* u = tests[i];
        printf("Query: %s\n", u);
        char* out = get_user_info(u);
        if (out) {
            fputs(out, stdout);
            if (out[0] == '\0') {
                puts("(no output or invalid username)");
            }
            free(out);
        } else {
            puts("(no output or invalid username)");
        }
        puts("-----");
    }
    return 0;
}