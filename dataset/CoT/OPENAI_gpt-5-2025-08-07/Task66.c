/* Chain-of-Through steps applied:
 * 1) Understand: Safely validate/execute limited commands, return output.
 * 2) Security: No system/popen; allowlist; forbid metachars; limit len/args/time/output.
 * 3) Implement: fork+execv absolute path; pipe capture; non-blocking read; timeout.
 * 4) Review: bounded buffers, checks, no shell, restricted args/paths.
 * 5) Finalize: Function and 5 tests.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <time.h>

#define MAX_CMD_LEN_C 256
#define MAX_ARGS_C 8
#define MAX_OUTPUT_BYTES_C 1000000
#define TIMEOUT_SEC_C 3

static int has_forbidden_chars(const char* s) {
    const char* forb = ";|&><`$(){}[]\\'\"*!?~\r\n\t";
    for (const unsigned char* p = (const unsigned char*)s; *p; ++p) {
        if (*p < 32 || *p > 126) return 1;
        if (strchr(forb, *p) != NULL) return 1;
    }
    return 0;
}

static int is_safe_token(const char* t) {
    size_t len = strlen(t);
    if (len == 0 || len > 128) return 0;
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)t[i];
        if (c < 32 || c > 126) return 0;
        if (strchr(";|&><`$(){}[]\\'\"*!?~\r\n\t", c) != NULL) return 0;
        if (!(isalnum(c) || c=='.' || c=='_' || c=='/' || c=='-' ||
              c=='=' || c==':' || c=='@' || c==',' || c=='+' || c=='%')) {
            return 0;
        }
    }
    return 1;
}

static int is_safe_path_arg(const char* t) {
    if (!is_safe_token(t)) return 0;
    if (t[0] == '/') return 0;
    if (t[0] == '-') return 0;
    if (strstr(t, "..") != NULL) return 0;
    return 1;
}

static int is_ls_flag(const char* t) {
    size_t len = strlen(t);
    if (len < 2 || t[0] != '-') return 0;
    for (size_t i = 1; i < len; ++i) {
        char c = t[i];
        if (strchr("lah1", c) == NULL) return 0;
    }
    return 1;
}

static char** tokenize(const char* s, int* out_count) {
    // Simple whitespace split
    char* tmp = strdup(s);
    if (!tmp) return NULL;
    char** tokens = (char**)calloc(MAX_ARGS_C + 1, sizeof(char*));
    if (!tokens) { free(tmp); return NULL; }
    int count = 0;
    char* saveptr = NULL;
    char* tok = strtok_r(tmp, " \t", &saveptr);
    while (tok && count < MAX_ARGS_C) {
        tokens[count++] = strdup(tok);
        tok = strtok_r(NULL, " \t", &saveptr);
    }
    // If more tokens exist beyond limit, invalid
    if (tok != NULL) {
        for (int i = 0; i < count; ++i) free(tokens[i]);
        free(tokens);
        free(tmp);
        return NULL;
    }
    *out_count = count;
    free(tmp);
    return tokens;
}

static void free_tokens(char** tokens, int count) {
    if (!tokens) return;
    for (int i = 0; i < count; ++i) free(tokens[i]);
    free(tokens);
}

static int in_allowed(const char* cmd) {
    const char* allowed[] = {"echo","ls","pwd","whoami","date","uname","cat"};
    for (size_t i = 0; i < sizeof(allowed)/sizeof(allowed[0]); ++i) {
        if (strcmp(cmd, allowed[i]) == 0) return 1;
    }
    return 0;
}

static int validate_tokens(char** tokens, int count) {
    if (count <= 0 || count > MAX_ARGS_C) return 0;
    const char* cmd = tokens[0];
    if (!in_allowed(cmd)) return 0;
    int argc = count - 1;
    char** args = tokens + 1;

    if (strcmp(cmd, "pwd") == 0 || strcmp(cmd, "whoami") == 0 || strcmp(cmd, "date") == 0) {
        return argc == 0;
    } else if (strcmp(cmd, "uname") == 0) {
        return argc == 0 || (argc == 1 && strcmp(args[0], "-a") == 0);
    } else if (strcmp(cmd, "echo") == 0) {
        if (argc > 5) return 0;
        for (int i = 0; i < argc; ++i) if (!is_safe_token(args[i])) return 0;
        return 1;
    } else if (strcmp(cmd, "ls") == 0) {
        if (argc > 2) return 0;
        int sawPath = 0;
        for (int i = 0; i < argc; ++i) {
            if (args[i][0] == '-') {
                if (!is_ls_flag(args[i])) return 0;
            } else {
                if (sawPath) return 0;
                if (!is_safe_path_arg(args[i])) return 0;
                sawPath = 1;
            }
        }
        return 1;
    } else if (strcmp(cmd, "cat") == 0) {
        if (argc == 0 || argc > 2) return 0;
        for (int i = 0; i < argc; ++i) if (!is_safe_path_arg(args[i])) return 0;
        return 1;
    }
    return 0;
}

static char* resolve_absolute(const char* cmd) {
    const char* bases[] = {"/bin/", "/usr/bin/"};
    for (size_t i = 0; i < sizeof(bases)/sizeof(bases[0]); ++i) {
        char path[512];
        snprintf(path, sizeof(path), "%s%s", bases[i], cmd);
        if (access(path, X_OK) == 0) {
            return strdup(path);
        }
    }
    return NULL;
}

static char* read_nonblocking_output(int fd, pid_t childPid) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags >= 0) fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    size_t cap = 8192;
    size_t len = 0;
    char* out = (char*)malloc(cap);
    if (!out) return NULL;

    time_t deadline = time(NULL) + TIMEOUT_SEC_C;
    int childExited = 0;

    while (1) {
        char buf[4096];
        ssize_t n;
        do {
            n = read(fd, buf, sizeof(buf));
            if (n > 0) {
                if (len + (size_t)n + 1 > cap) {
                    size_t newcap = cap * 2;
                    if (newcap < len + (size_t)n + 1) newcap = len + (size_t)n + 1;
                    char* tmp = (char*)realloc(out, newcap);
                    if (!tmp) {
                        // OOM, kill child and return what we have
                        kill(childPid, SIGKILL);
                        int status;
                        waitpid(childPid, &status, 0);
                        out[len] = '\0';
                        return out;
                    }
                    out = tmp;
                    cap = newcap;
                }
                if (len + (size_t)n > MAX_OUTPUT_BYTES_C) {
                    const char* trunc = "\n[Truncated output]";
                    size_t tlen = strlen(trunc);
                    size_t space = MAX_OUTPUT_BYTES_C > len ? MAX_OUTPUT_BYTES_C - len : 0;
                    memcpy(out + len, buf, space);
                    len += space;
                    if (len + tlen + 1 > cap) {
                        char* tmp = (char*)realloc(out, len + tlen + 1);
                        if (!tmp) { /* ignore */ }
                        else { out = tmp; cap = len + tlen + 1; }
                    }
                    memcpy(out + len, trunc, tlen);
                    len += tlen;
                    out[len] = '\0';
                    kill(childPid, SIGKILL);
                    int status;
                    waitpid(childPid, &status, 0);
                    return out;
                }
                memcpy(out + len, buf, (size_t)n);
                len += (size_t)n;
            }
        } while (n > 0);

        int status = 0;
        pid_t r = waitpid(childPid, &status, WNOHANG);
        if (r == childPid) {
            childExited = 1;
            // drain remaining
            while ((n = read(fd, buf, sizeof(buf))) > 0) {
                if (len + (size_t)n + 1 > cap) {
                    size_t newcap = cap * 2;
                    if (newcap < len + (size_t)n + 1) newcap = len + (size_t)n + 1;
                    char* tmp = (char*)realloc(out, newcap);
                    if (!tmp) break;
                    out = tmp; cap = newcap;
                }
                if (len + (size_t)n > MAX_OUTPUT_BYTES_C) break;
                memcpy(out + len, buf, (size_t)n);
                len += (size_t)n;
            }
            break;
        }

        if (time(NULL) > deadline) {
            kill(childPid, SIGKILL);
            waitpid(childPid, &status, 0);
            const char* timeoutMsg = "[Error] Execution timed out";
            size_t tlen = strlen(timeoutMsg);
            if (len + tlen + 1 > cap) {
                char* tmp = (char*)realloc(out, len + tlen + 1);
                if (tmp) { out = tmp; cap = len + tlen + 1; }
            }
            memcpy(out + len, timeoutMsg, tlen);
            len += tlen;
            out[len] = '\0';
            return out;
        }
        usleep(20000);
    }
    out[len] = '\0';
    return out;
}

char* run_safe_command(const char* input) {
    if (!input || !*input) {
        return strdup("Error: Empty input");
    }
    if ((int)strlen(input) > MAX_CMD_LEN_C) {
        return strdup("Error: Command too long");
    }
    if (has_forbidden_chars(input)) {
        return strdup("Error: Forbidden/invalid characters present");
    }

    int count = 0;
    char** tokens = tokenize(input, &count);
    if (!tokens || count <= 0) {
        if (tokens) free_tokens(tokens, count);
        return strdup("Error: Tokenization failed or too many args");
    }

    if (!validate_tokens(tokens, count)) {
        free_tokens(tokens, count);
        return strdup("Error: Command not allowed or invalid arguments");
    }

    char* absbin = resolve_absolute(tokens[0]);
    if (!absbin) {
        free_tokens(tokens, count);
        return strdup("Error: Command binary not found");
    }
    free(tokens[0]);
    tokens[0] = absbin;

    int pipefd[2];
    if (pipe(pipefd) != 0) {
        free_tokens(tokens, count);
        return strdup("Error: Pipe failed");
    }

    pid_t pid = fork();
    if (pid < 0) {
        close(pipefd[0]); close(pipefd[1]);
        free_tokens(tokens, count);
        return strdup("Error: Fork failed");
    } else if (pid == 0) {
        // Child
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);

        // Build argv
        char** argv = (char**)calloc((size_t)count + 1, sizeof(char*));
        if (!argv) _exit(127);
        for (int i = 0; i < count; ++i) argv[i] = tokens[i];
        argv[count] = NULL;

        unsetenv("LD_PRELOAD");
        unsetenv("LD_LIBRARY_PATH");

        execv(argv[0], argv);
        const char* msg = "Execution failed\n";
        write(STDOUT_FILENO, msg, strlen(msg));
        _exit(127);
    } else {
        // Parent
        close(pipefd[1]);
        char* out = read_nonblocking_output(pipefd[0], pid);
        close(pipefd[0]);
        free_tokens(tokens, count);
        if (!out) return strdup("Error: Read failed");
        return out;
    }
}

int main(void) {
    const char* tests[] = {
        "pwd",
        "whoami",
        "date",
        "echo safe_test",
        "ls -l"
    };
    for (size_t i = 0; i < sizeof(tests)/sizeof(tests[0]); ++i) {
        printf(">>> %s\n", tests[i]);
        char* out = run_safe_command(tests[i]);
        if (out) {
            printf("%s\n", out);
            free(out);
        } else {
            printf("Error: no output\n");
        }
    }
    return 0;
}