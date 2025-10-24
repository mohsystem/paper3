#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

typedef struct {
    int exitCode;
    char *stdoutStr;
    char *stderrStr;
    char *error;
} ExecResult;

static int is_allowed_char(int c) {
    if (isalnum((unsigned char)c)) return 1;
    const char *allowed = "._-+/=%:,@";
    for (const char *p = allowed; *p; ++p) if (*p == c) return 1;
    return 0;
}

static int is_safe_token(const char *s, size_t maxLen) {
    if (s == NULL) return 0;
    size_t n = strlen(s);
    if (n == 0 || n > maxLen) return 0;
    for (size_t i = 0; i < n; ++i) {
        if (!is_allowed_char((unsigned char)s[i])) return 0;
    }
    return 1;
}

static int is_safe_path_token(const char *s) {
    if (!is_safe_token(s, 128)) return 0;
    if (strstr(s, "..") != NULL) return 0;
    if (s[0] == '/') return 0;
    return 1;
}

static char *strdup_safe(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s);
    char *p = (char*)malloc(n + 1);
    if (!p) return NULL;
    memcpy(p, s, n + 1);
    return p;
}

typedef struct {
    char **data;
    size_t size;
    size_t cap;
} StrVec;

static void sv_init(StrVec *v) {
    v->data = NULL; v->size = 0; v->cap = 0;
}
static void sv_free(StrVec *v) {
    for (size_t i = 0; i < v->size; ++i) free(v->data[i]);
    free(v->data);
    v->data = NULL; v->size = v->cap = 0;
}
static int sv_push(StrVec *v, const char *s) {
    if (v->size == v->cap) {
        size_t ncap = v->cap ? v->cap * 2 : 8;
        char **nd = (char**)realloc(v->data, ncap * sizeof(char*));
        if (!nd) return 0;
        v->data = nd; v->cap = ncap;
    }
    v->data[v->size] = strdup_safe(s);
    if (!v->data[v->size]) return 0;
    v->size++;
    return 1;
}

static int split_tokens(const char *input, StrVec *tokens, char **err) {
    tokens->size = 0;
    if (!input || input[0] == '\0') { *err = strdup_safe("Empty input."); return 0; }
    if (strlen(input) > 256) { *err = strdup_safe("Input too long."); return 0; }

    char *buf = strdup_safe(input);
    if (!buf) { *err = strdup_safe("Out of memory."); return 0; }

    char *save = NULL;
    char *tok = strtok_r(buf, " \t\r\n", &save);
    while (tok) {
        if (tokens->size >= 10) { free(buf); *err = strdup_safe("Too many arguments (max 10)."); return 0; }
        if (!sv_push(tokens, tok)) { free(buf); *err = strdup_safe("Out of memory."); return 0; }
        tok = strtok_r(NULL, " \t\r\n", &save);
    }
    free(buf);
    if (tokens->size == 0) { *err = strdup_safe("No tokens found."); return 0; }
    return 1;
}

static const char *basename_of(const char *path) {
    const char *slash = strrchr(path, '/');
    if (!slash) return path;
    if (*(slash + 1) == '\0') return ""; // trailing slash (invalid for our cases)
    return slash + 1;
}

static int validate_and_build_argv(const char *input, StrVec *argvOut, char **err) {
    sv_init(argvOut);
    StrVec tokens; sv_init(&tokens);
    if (!split_tokens(input, &tokens, err)) { sv_free(&tokens); return 0; }

    const char *cmd = tokens.data[0];
    const char *base = basename_of(cmd);

    const char *cmd_echo   = "/bin/echo";
    const char *cmd_ls     = "/bin/ls";
    const char *cmd_uname  = "/bin/uname";
    const char *cmd_id     = "/usr/bin/id";
    const char *cmd_date   = "/bin/date";
    const char *cmd_whoami = "/usr/bin/whoami";

    const char *execPath = NULL;
    if (strcmp(base, "echo") == 0) execPath = cmd_echo;
    else if (strcmp(base, "ls") == 0) execPath = cmd_ls;
    else if (strcmp(base, "uname") == 0) execPath = cmd_uname;
    else if (strcmp(base, "id") == 0) execPath = cmd_id;
    else if (strcmp(base, "date") == 0) execPath = cmd_date;
    else if (strcmp(base, "whoami") == 0) execPath = cmd_whoami;
    else { *err = strdup_safe("Command not allowed."); sv_free(&tokens); return 0; }

    // basic token safety
    for (size_t i = 0; i < tokens.size; ++i) {
        size_t lim = (i == 0) ? 64 : 128;
        if (!is_safe_token(tokens.data[i], lim)) {
            *err = strdup_safe("Token contains disallowed characters or is too long.");
            sv_free(&tokens);
            return 0;
        }
    }

    if (!sv_push(argvOut, execPath)) { *err = strdup_safe("Out of memory."); sv_free(&tokens); return 0; }

    if (execPath == cmd_echo) {
        for (size_t i = 1; i < tokens.size; ++i) {
            if (!sv_push(argvOut, tokens.data[i])) { *err = strdup_safe("Out of memory."); sv_free(argvOut); sv_init(argvOut); sv_free(&tokens); return 0; }
        }
    } else if (execPath == cmd_ls) {
        for (size_t i = 1; i < tokens.size; ++i) {
            const char *t = tokens.data[i];
            if (t[0] == '-') {
                if (!(strcmp(t, "-l") == 0 || strcmp(t, "-a") == 0 || strcmp(t, "-al") == 0 || strcmp(t, "-la") == 0)) {
                    *err = strdup_safe("Unsupported ls option.");
                    sv_free(argvOut); sv_init(argvOut); sv_free(&tokens); return 0;
                }
                if (!sv_push(argvOut, t)) { *err = strdup_safe("Out of memory."); sv_free(argvOut); sv_init(argvOut); sv_free(&tokens); return 0; }
            } else {
                if (!is_safe_path_token(t)) { *err = strdup_safe("Unsafe ls path argument."); sv_free(argvOut); sv_init(argvOut); sv_free(&tokens); return 0; }
                if (!sv_push(argvOut, t)) { *err = strdup_safe("Out of memory."); sv_free(argvOut); sv_init(argvOut); sv_free(&tokens); return 0; }
            }
        }
    } else if (execPath == cmd_uname) {
        for (size_t i = 1; i < tokens.size; ++i) {
            const char *t = tokens.data[i];
            if (t[0] != '-') { *err = strdup_safe("uname only accepts options."); sv_free(argvOut); sv_init(argvOut); sv_free(&tokens); return 0; }
            if (!(strcmp(t, "-a") == 0 || strcmp(t, "-s") == 0 || strcmp(t, "-r") == 0 || strcmp(t, "-m") == 0 ||
                  strcmp(t, "-n") == 0 || strcmp(t, "-v") == 0 || strcmp(t, "-p") == 0 || strcmp(t, "-o") == 0)) {
                *err = strdup_safe("Unsupported uname option.");
                sv_free(argvOut); sv_init(argvOut); sv_free(&tokens); return 0;
            }
            if (!sv_push(argvOut, t)) { *err = strdup_safe("Out of memory."); sv_free(argvOut); sv_init(argvOut); sv_free(&tokens); return 0; }
        }
    } else if (execPath == cmd_id) {
        for (size_t i = 1; i < tokens.size; ++i) {
            const char *t = tokens.data[i];
            if (t[0] != '-') { *err = strdup_safe("id only accepts options."); sv_free(argvOut); sv_init(argvOut); sv_free(&tokens); return 0; }
            if (!(strcmp(t, "-u") == 0 || strcmp(t, "-g") == 0)) {
                *err = strdup_safe("Unsupported id option.");
                sv_free(argvOut); sv_init(argvOut); sv_free(&tokens); return 0;
            }
            if (!sv_push(argvOut, t)) { *err = strdup_safe("Out of memory."); sv_free(argvOut); sv_init(argvOut); sv_free(&tokens); return 0; }
        }
    } else if (execPath == cmd_date) {
        for (size_t i = 1; i < tokens.size; ++i) {
            const char *t = tokens.data[i];
            if (t[0] == '-') {
                if (strcmp(t, "-u") != 0) { *err = strdup_safe("Unsupported date option."); sv_free(argvOut); sv_init(argvOut); sv_free(&tokens); return 0; }
                if (!sv_push(argvOut, t)) { *err = strdup_safe("Out of memory."); sv_free(argvOut); sv_init(argvOut); sv_free(&tokens); return 0; }
            } else if (t[0] == '+') {
                size_t len = strlen(t);
                if (len > 64) { *err = strdup_safe("date format too long."); sv_free(argvOut); sv_init(argvOut); sv_free(&tokens); return 0; }
                int ok = 1;
                for (size_t k = 1; k < len; ++k) {
                    char c = t[k];
                    if (!(isalnum((unsigned char)c) || c == '%' || c == ':' || c == '.' || c == '/' || c == '_' || c == '-')) {
                        ok = 0; break;
                    }
                }
                if (!ok) { *err = strdup_safe("date format contains disallowed characters."); sv_free(argvOut); sv_init(argvOut); sv_free(&tokens); return 0; }
                if (!sv_push(argvOut, t)) { *err = strdup_safe("Out of memory."); sv_free(argvOut); sv_init(argvOut); sv_free(&tokens); return 0; }
            } else {
                *err = strdup_safe("Unsupported date argument.");
                sv_free(argvOut); sv_init(argvOut); sv_free(&tokens); return 0;
            }
        }
    } else if (execPath == cmd_whoami) {
        if (tokens.size != 1) { *err = strdup_safe("whoami does not accept arguments."); sv_free(argvOut); sv_init(argvOut); sv_free(&tokens); return 0; }
    } else {
        *err = strdup_safe("Internal error.");
        sv_free(argvOut); sv_init(argvOut); sv_free(&tokens); return 0;
    }

    sv_free(&tokens);
    return 1;
}

static int set_cloexec(int fd) {
    int flags = fcntl(fd, F_GETFD);
    if (flags == -1) return 0;
    if (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1) return 0;
    return 1;
}

static char *read_all_fd_limited(int fd, size_t maxBytes) {
    size_t cap = 4096;
    if (cap > maxBytes) cap = maxBytes;
    char *buf = (char*)malloc(cap + 1);
    if (!buf) return NULL;
    size_t len = 0;

    while (1) {
        if (len == cap) {
            if (cap >= maxBytes) break;
            size_t ncap = cap * 2;
            if (ncap > maxBytes) ncap = maxBytes;
            char *nb = (char*)realloc(buf, ncap + 1);
            if (!nb) break;
            buf = nb; cap = ncap;
        }
        ssize_t n = read(fd, buf + len, cap - len);
        if (n > 0) {
            len += (size_t)n;
        } else if (n == 0) {
            break;
        } else {
            if (errno == EINTR) continue;
            break;
        }
    }
    buf[len] = '\0';
    return buf;
}

static void free_exec_result(ExecResult *r) {
    if (!r) return;
    free(r->stdoutStr); r->stdoutStr = NULL;
    free(r->stderrStr); r->stderrStr = NULL;
    free(r->error); r->error = NULL;
}

static ExecResult run_command_argv(StrVec *argvIn) {
    ExecResult res;
    res.exitCode = -1;
    res.stdoutStr = NULL;
    res.stderrStr = NULL;
    res.error = NULL;

    int outPipe[2] = {-1,-1};
    int errPipe[2] = {-1,-1};
    if (pipe(outPipe) == -1 || pipe(errPipe) == -1) {
        res.error = strdup_safe("Failed to create pipes.");
        if (outPipe[0] != -1) { close(outPipe[0]); close(outPipe[1]); }
        if (errPipe[0] != -1) { close(errPipe[0]); close(errPipe[1]); }
        return res;
    }
    set_cloexec(outPipe[0]); set_cloexec(outPipe[1]);
    set_cloexec(errPipe[0]); set_cloexec(errPipe[1]);

    pid_t pid = fork();
    if (pid == -1) {
        res.error = strdup_safe("fork failed.");
        close(outPipe[0]); close(outPipe[1]);
        close(errPipe[0]); close(errPipe[1]);
        return res;
    }
    if (pid == 0) {
        // Child
        (void)dup2(outPipe[1], STDOUT_FILENO);
        (void)dup2(errPipe[1], STDERR_FILENO);
        close(outPipe[0]); close(outPipe[1]);
        close(errPipe[0]); close(errPipe[1]);

        // Build argv array for execv
        size_t argc = argvIn->size;
        char **argv = (char**)calloc(argc + 1, sizeof(char*));
        if (!argv) _exit(127);
        for (size_t i = 0; i < argc; ++i) {
            argv[i] = argvIn->data[i];
        }
        argv[argc] = NULL;

        execv(argvIn->data[0], argv);
        const char msg[] = "exec failed\n";
        write(STDERR_FILENO, msg, sizeof(msg)-1);
        _exit(127);
    }

    // Parent
    close(outPipe[1]);
    close(errPipe[1]);

    char *outStr = read_all_fd_limited(outPipe[0], 65536);
    char *errStr = read_all_fd_limited(errPipe[0], 65536);

    close(outPipe[0]);
    close(errPipe[0]);

    int status = 0;
    if (waitpid(pid, &status, 0) == -1) {
        res.error = strdup_safe("waitpid failed.");
        free(outStr); free(errStr);
        res.exitCode = -1;
        return res;
    }
    if (WIFEXITED(status)) res.exitCode = WEXITSTATUS(status);
    else if (WIFSIGNALED(status)) res.exitCode = 128 + WTERMSIG(status);
    else res.exitCode = -1;

    res.stdoutStr = outStr ? outStr : strdup_safe("");
    res.stderrStr = errStr ? errStr : strdup_safe("");
    return res;
}

static char *execute_validated_command(const char *input) {
    StrVec argv; sv_init(&argv);
    char *err = NULL;
    if (!validate_and_build_argv(input, &argv, &err)) {
        size_t n = strlen(err ? err : "error") + 8;
        char *msg = (char*)malloc(n);
        if (!msg) { free(err); return strdup_safe("error: OOM\n"); }
        snprintf(msg, n, "error: %s\n", err ? err : "error");
        free(err);
        return msg;
    }
    ExecResult r = run_command_argv(&argv);
    sv_free(&argv);

    if (r.error) {
        size_t n = strlen(r.error) + 8;
        char *msg = (char*)malloc(n);
        if (!msg) { free_exec_result(&r); return strdup_safe("error: OOM\n"); }
        snprintf(msg, n, "error: %s\n", r.error);
        free_exec_result(&r);
        return msg;
    }

    // Build output string
    const char *prefix1 = "exit=";
    const char *prefix2 = "\nstdout:\n";
    const char *prefix3 = "\nstderr:\n";
    size_t need = strlen(prefix1) + 10 + strlen(prefix2) + strlen(r.stdoutStr) + strlen(prefix3) + strlen(r.stderrStr) + 2;
    char *out = (char*)malloc(need);
    if (!out) { free_exec_result(&r); return strdup_safe("error: OOM\n"); }
    snprintf(out, need, "%sexit%d%s%s%s%s",
             "", r.exitCode, prefix2, r.stdoutStr, prefix3, r.stderrStr);
    free_exec_result(&r);
    return out;
}

static void run_test(const char *cmd) {
    printf("Test: %s\n", cmd);
    char *res = execute_validated_command(cmd);
    if (res) {
        fputs(res, stdout);
        if (res[strlen(res)-1] != '\n') fputc('\n', stdout);
        free(res);
    }
    fputc('\n', stdout);
}

int main(void) {
    // 5 test cases
    run_test("echo Hello_World");
    run_test("ls -l");
    run_test("uname -a");
    run_test("id -u");
    run_test("date +%Y-%m-%d");

    // Read a command from user input
    char line[512];
    printf("Enter a command: ");
    if (fgets(line, sizeof(line), stdin) != NULL) {
        // strip newline
        size_t n = strlen(line);
        if (n > 0 && (line[n-1] == '\n' || line[n-1] == '\r')) line[n-1] = '\0';
        char *res = execute_validated_command(line);
        if (res) {
            fputs(res, stdout);
            if (res[strlen(res)-1] != '\n') fputc('\n', stdout);
            free(res);
        }
    }
    return 0;
}