// Task39 - C
// Chain-of-Through in comments:
// 1) Implement: function that runs system command to display file contents.
// 2) Security: validate filename; ensure regular file; avoid shell; use /bin/cat with "--".
// 3) Secure coding: fork+exec with pipe; whitelist characters; check errors.
// 4) Review: manage resources; avoid injection; handle timeouts omitted for simplicity.
// 5) Output: final code with 5 tests.
//
// Note: This C implementation targets POSIX systems.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_FILENAME_LEN_C 4096

static int is_regular_file_c(const char* path) {
    struct stat st;
    if (stat(path, &st) != 0) return 0;
    return S_ISREG(st.st_mode);
}

static int is_safe_filename_c(const char* filename) {
    if (!filename) return 0;
    size_t len = strlen(filename);
    if (len == 0 || len > MAX_FILENAME_LEN_C) return 0;
    if (memchr(filename, '\0', len) == NULL) {
        // Normal C strings end with '\0', just a sanity check in case of non-terminated input.
    }
    // Whitelist pattern
    regex_t re;
    if (regcomp(&re, "^[A-Za-z0-9_./\\\\ \\-:]+$", REG_EXTENDED | REG_NOSUB) != 0) return 0;
    int ok = regexec(&re, filename, 0, NULL, 0) == 0;
    regfree(&re);
    return ok;
}

char* display_file_secure(const char* filename) {
    if (!is_safe_filename_c(filename)) {
        char* msg = strdup("Error: Invalid filename.");
        return msg ? msg : NULL;
    }
    if (!is_regular_file_c(filename)) {
        char* msg = strdup("Error: File not found or not a regular file.");
        return msg ? msg : NULL;
    }

    int pipefd[2];
    if (pipe(pipefd) != 0) {
        char* msg = strdup("Error: Failed to create pipe.");
        return msg ? msg : NULL;
    }

    pid_t pid = fork();
    if (pid < 0) {
        close(pipefd[0]);
        close(pipefd[1]);
        char* msg = strdup("Error: Failed to fork.");
        return msg ? msg : NULL;
    } else if (pid == 0) {
        // Child: exec cat -- filename
        close(pipefd[0]);
        if (dup2(pipefd[1], STDOUT_FILENO) == -1) _exit(127);
        if (dup2(pipefd[1], STDERR_FILENO) == -1) _exit(127);
        close(pipefd[1]);
        const char* prog = "/bin/cat";
        char* const args[] = {(char*)"cat", (char*)"--", (char*)filename, NULL};
        execv(prog, args);
        _exit(127);
    } else {
        // Parent
        close(pipefd[1]);
        size_t cap = 4096;
        size_t len = 0;
        char* out = (char*)malloc(cap);
        if (!out) {
            close(pipefd[0]);
            return NULL;
        }
        ssize_t n;
        char buf[4096];
        while ((n = read(pipefd[0], buf, sizeof(buf))) > 0) {
            if (len + (size_t)n + 1 > cap) {
                size_t newcap = (cap * 2 > len + (size_t)n + 1) ? cap * 2 : (len + (size_t)n + 1);
                char* tmp = (char*)realloc(out, newcap);
                if (!tmp) {
                    free(out);
                    close(pipefd[0]);
                    return NULL;
                }
                cap = newcap;
                out = tmp;
            }
            memcpy(out + len, buf, (size_t)n);
            len += (size_t)n;
        }
        close(pipefd[0]);
        int status = 0;
        waitpid(pid, &status, 0);
        // Null-terminate
        if (len + 1 > cap) {
            char* tmp = (char*)realloc(out, len + 1);
            if (!tmp) {
                free(out);
                return NULL;
            }
            out = tmp;
        }
        out[len] = '\0';

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            return out;
        } else {
            // Build error message
            const char* prefix = "Error: Command failed with exit code ";
            char codebuf[32];
            int code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
            snprintf(codebuf, sizeof(codebuf), "%d", code);
            const char* mid = ". Output:\n";
            size_t mlen = strlen(prefix) + strlen(codebuf) + strlen(mid) + strlen(out) + 1;
            char* msg = (char*)malloc(mlen);
            if (!msg) {
                free(out);
                return NULL;
            }
            snprintf(msg, mlen, "%s%s%s%s", prefix, codebuf, mid, out);
            free(out);
            return msg;
        }
    }
}

static char* join_path_c(const char* a, const char* b) {
    size_t la = strlen(a), lb = strlen(b);
    int need_slash = (la > 0 && a[la - 1] != '/');
    size_t total = la + (need_slash ? 1 : 0) + lb + 1;
    char* res = (char*)malloc(total);
    if (!res) return NULL;
    strcpy(res, a);
    if (need_slash) strcat(res, "/");
    strcat(res, b);
    return res;
}

static int write_file_in_dir_c(const char* dir, const char* name, const char* content, char** out_path) {
    *out_path = join_path_c(dir, name);
    if (!*out_path) return -1;
    int fd = open(*out_path, O_CREAT | O_TRUNC | O_WRONLY, 0600);
    if (fd < 0) return -1;
    size_t len = strlen(content);
    ssize_t off = 0;
    while ((size_t)off < len) {
        ssize_t w = write(fd, content + off, len - (size_t)off);
        if (w <= 0) break;
        off += w;
    }
    close(fd);
    return 0;
}

int main(void) {
    // Temp directory
    char tmpl[] = "/tmp/task39-c-XXXXXX";
    char* dir = mkdtemp(tmpl);
    if (!dir) {
        fprintf(stderr, "Failed to create temp dir\n");
        return 1;
    }

    char* f1; write_file_in_dir_c(dir, "file1.txt", "Hello from file1\nLine2\n", &f1);
    char* f2; write_file_in_dir_c(dir, "file with spaces.txt", "Content with spaces in filename\n", &f2);
    char* f3; write_file_in_dir_c(dir, "-leadingDash.txt", "Filename starts with dash\n", &f3);
    char* f4; write_file_in_dir_c(dir, "normal-_.txt", "Normal file name content\n", &f4);
    char* f5 = join_path_c(dir, "does_not_exist.txt");

    char* o1 = display_file_secure(f1);
    printf("Test1:\n%s", o1 ? o1 : "Error: NULL\n");
    free(o1);

    char* o2 = display_file_secure(f2);
    printf("Test2:\n%s", o2 ? o2 : "Error: NULL\n");
    free(o2);

    char* o3 = display_file_secure(f3);
    printf("Test3:\n%s", o3 ? o3 : "Error: NULL\n");
    free(o3);

    char* o4 = display_file_secure(f4);
    printf("Test4:\n%s", o4 ? o4 : "Error: NULL\n");
    free(o4);

    char* o5 = display_file_secure(f5);
    printf("Test5:\n%s", o5 ? o5 : "Error: NULL\n");
    free(o5);

    free(f1); free(f2); free(f3); free(f4); free(f5);
    return 0;
}