#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <spawn.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>

extern char **environ;

#define MAX_PATH_LEN 4096
#define MAX_OUTPUT_SIZE (10 * 1024 * 1024)

typedef struct {
    int ok;        // 1 on success, 0 on error
    char *output;  // malloc'd output buffer on success (may contain NUL bytes)
    size_t out_len;
    char *error;   // malloc'd error message on failure
} CatResult;

static void free_result(CatResult *r) {
    if (!r) return;
    free(r->output);
    free(r->error);
    r->output = NULL;
    r->error = NULL;
    r->out_len = 0;
    r->ok = 0;
}

static char* dup_err(const char* msg) {
    size_t n = strlen(msg);
    char *p = (char*)malloc(n + 1);
    if (!p) return NULL;
    memcpy(p, msg, n + 1);
    return p;
}

static int is_valid_component(const char *comp) {
    if (!comp || comp[0] == '\0') return 0;
    if (strcmp(comp, ".") == 0 || strcmp(comp, "..") == 0) return 0;
    for (const unsigned char *u = (const unsigned char*)comp; *u; ++u) {
        if (*u < 0x20) return 0;
    }
    return 1;
}

static int is_valid_user_path(const char *p) {
    if (!p) return 0;
    size_t len = strnlen(p, MAX_PATH_LEN + 1);
    if (len == 0 || len > MAX_PATH_LEN) return 0;
    if (p[0] == '/') return 0;
    // Ensure at least one valid component and no "." or ".."
    char *tmp = (char*)malloc(len + 1);
    if (!tmp) return 0;
    memcpy(tmp, p, len + 1);
    int had = 0;
    char *saveptr = NULL;
    char *tok = strtok_r(tmp, "/", &saveptr);
    while (tok) {
        if (!is_valid_component(tok)) { free(tmp); return 0; }
        had = 1;
        tok = strtok_r(NULL, "/", &saveptr);
    }
    free(tmp);
    return had;
}

static int pick_cat_path(char *out, size_t outsz) {
    const char* candidates[] = {"/bin/cat", "/usr/bin/cat"};
    for (size_t i = 0; i < sizeof(candidates)/sizeof(candidates[0]); ++i) {
        if (access(candidates[i], X_OK) == 0) {
            snprintf(out, outsz, "%s", candidates[i]);
            return 1;
        }
    }
    return 0;
}

static int open_under_base(const char *base_dir, const char *user_path, int *out_fd, char **err) {
    *out_fd = -1;
    if (!is_valid_user_path(user_path)) {
        *err = dup_err("Invalid user path");
        return 0;
    }
    char base_real[PATH_MAX];
    if (!realpath(base_dir, base_real)) {
        char buf[256];
        snprintf(buf, sizeof(buf), "Failed to resolve base dir: %s", strerror(errno));
        *err = dup_err(buf);
        return 0;
    }
    int base_fd = open(base_real, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (base_fd < 0) {
        char buf[256];
        snprintf(buf, sizeof(buf), "Failed to open base dir: %s", strerror(errno));
        *err = dup_err(buf);
        return 0;
    }

    // Duplicate user_path to tokenize
    size_t up_len = strlen(user_path);
    char *tmp = (char*)malloc(up_len + 1);
    if (!tmp) {
        *err = dup_err("Out of memory");
        close(base_fd);
        return 0;
    }
    memcpy(tmp, user_path, up_len + 1);

    int dir_fd = base_fd;
    base_fd = -1; // ownership transferred
    char *saveptr = NULL;
    char *tok = strtok_r(tmp, "/", &saveptr);
    char *last = NULL;
    // Gather components: process intermediate dirs first
    char **comps = NULL;
    size_t comps_count = 0, comps_cap = 0;
    while (tok) {
        if (comps_count == comps_cap) {
            size_t nc = comps_cap ? comps_cap * 2 : 8;
            char **newv = (char**)realloc(comps, nc * sizeof(char*));
            if (!newv) {
                *err = dup_err("Out of memory");
                free(tmp);
                close(dir_fd);
                free(comps);
                return 0;
            }
            comps = newv;
            comps_cap = nc;
        }
        comps[comps_count++] = tok;
        tok = strtok_r(NULL, "/", &saveptr);
    }

    // Open intermediate directories with O_NOFOLLOW
    for (size_t i = 0; i + 1 < comps_count; ++i) {
        int next_fd = openat(dir_fd, comps[i], O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
        if (next_fd < 0) {
            char buf[256];
            snprintf(buf, sizeof(buf), "Failed to open directory '%s': %s", comps[i], strerror(errno));
            *err = dup_err(buf);
            free(tmp);
            free(comps);
            close(dir_fd);
            return 0;
        }
        close(dir_fd);
        dir_fd = next_fd;
    }

    last = comps_count ? comps[comps_count - 1] : NULL;
    if (!last) {
        *err = dup_err("Empty path after normalization");
        free(tmp);
        free(comps);
        close(dir_fd);
        return 0;
    }

    int file_fd = openat(dir_fd, last, O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
    if (file_fd < 0) {
        char buf[256];
        snprintf(buf, sizeof(buf), "Failed to open file: %s", strerror(errno));
        *err = dup_err(buf);
        free(tmp);
        free(comps);
        close(dir_fd);
        return 0;
    }
    struct stat st;
    if (fstat(file_fd, &st) != 0) {
        char buf[256];
        snprintf(buf, sizeof(buf), "fstat failed: %s", strerror(errno));
        *err = dup_err(buf);
        free(tmp);
        free(comps);
        close(dir_fd);
        close(file_fd);
        return 0;
    }
    if (!S_ISREG(st.st_mode)) {
        *err = dup_err("Target is not a regular file");
        free(tmp);
        free(comps);
        close(dir_fd);
        close(file_fd);
        return 0;
    }

    free(tmp);
    free(comps);
    close(dir_fd);
    *out_fd = file_fd;
    return 1;
}

CatResult display_file_via_command(const char *base_dir, const char *user_path) {
    CatResult res;
    res.ok = 0;
    res.output = NULL;
    res.out_len = 0;
    res.error = NULL;

    int file_fd = -1;
    if (!open_under_base(base_dir, user_path, &file_fd, &res.error)) {
        return res;
    }

    char cat_path[128];
    if (!pick_cat_path(cat_path, sizeof(cat_path))) {
        res.error = dup_err("cat executable not found at /bin/cat or /usr/bin/cat");
        close(file_fd);
        return res;
    }

    int pipefd[2];
#if defined(O_CLOEXEC)
    if (pipe2(pipefd, O_CLOEXEC) != 0) {
#else
    if (pipe(pipefd) != 0) {
#endif
        char buf[128];
        snprintf(buf, sizeof(buf), "pipe failed: %s", strerror(errno));
        res.error = dup_err(buf);
        close(file_fd);
        return res;
    }
#ifndef O_CLOEXEC
    fcntl(pipefd[0], F_SETFD, fcntl(pipefd[0], F_GETFD) | FD_CLOEXEC);
    fcntl(pipefd[1], F_SETFD, fcntl(pipefd[1], F_GETFD) | FD_CLOEXEC);
#endif

    posix_spawn_file_actions_t actions;
    if (posix_spawn_file_actions_init(&actions) != 0) {
        res.error = dup_err("posix_spawn_file_actions_init failed");
        close(file_fd);
        close(pipefd[0]); close(pipefd[1]);
        return res;
    }

    posix_spawn_file_actions_adddup2(&actions, file_fd, STDIN_FILENO);
    posix_spawn_file_actions_adddup2(&actions, pipefd[1], STDOUT_FILENO);
    posix_spawn_file_actions_addclose(&actions, pipefd[0]);
    posix_spawn_file_actions_addclose(&actions, pipefd[1]);
    posix_spawn_file_actions_addclose(&actions, file_fd);

    char *argv[] = {"cat", "-", NULL};
    pid_t pid = -1;
    int sp = posix_spawn(&pid, cat_path, &actions, NULL, argv, environ);
    posix_spawn_file_actions_destroy(&actions);
    close(file_fd);
    close(pipefd[1]); // parent closes write end

    if (sp != 0) {
        char buf[128];
        snprintf(buf, sizeof(buf), "posix_spawn failed: %s", strerror(sp));
        res.error = dup_err(buf);
        close(pipefd[0]);
        return res;
    }

    // Read from pipe into dynamic buffer with cap
    size_t cap = 8192;
    size_t len = 0;
    char *out = (char*)malloc(cap);
    if (!out) {
        res.error = dup_err("Out of memory");
        close(pipefd[0]);
        kill(pid, SIGKILL);
        waitpid(pid, NULL, 0);
        return res;
    }

    char buf[8192];
    ssize_t r;
    int read_err = 0;
    while ((r = read(pipefd[0], buf, sizeof(buf))) > 0) {
        if (len + (size_t)r > MAX_OUTPUT_SIZE) {
            res.error = dup_err("Output exceeds maximum allowed size");
            read_err = 1;
            break;
        }
        if (len + (size_t)r > cap) {
            size_t ncap = cap * 2;
            while (ncap < len + (size_t)r) ncap *= 2;
            char *n = (char*)realloc(out, ncap);
            if (!n) {
                res.error = dup_err("Out of memory");
                read_err = 1;
                break;
            }
            out = n;
            cap = ncap;
        }
        memcpy(out + len, buf, (size_t)r);
        len += (size_t)r;
    }
    int savedErrno = errno;
    close(pipefd[0]);

    if (r < 0 && !read_err) {
        char e[128];
        snprintf(e, sizeof(e), "read failed: %s", strerror(savedErrno));
        res.error = dup_err(e);
        read_err = 1;
    }

    int status = 0;
    if (waitpid(pid, &status, 0) < 0 && !read_err) {
        char e[128];
        snprintf(e, sizeof(e), "waitpid failed: %s", strerror(errno));
        res.error = dup_err(e);
        read_err = 1;
    } else if (!read_err && (!WIFEXITED(status) || WEXITSTATUS(status) != 0)) {
        res.error = dup_err("cat process failed");
        read_err = 1;
    }

    if (read_err) {
        free(out);
        return res;
    }

    res.ok = 1;
    res.output = out;
    res.out_len = len;
    res.error = NULL;
    return res;
}

// Helper to write a file safely
static int write_file(const char *path, const char *data, size_t n, char **err) {
    int fd = open(path, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0600);
    if (fd < 0) {
        char b[256];
        snprintf(b, sizeof(b), "open for write failed: %s", strerror(errno));
        *err = dup_err(b);
        return 0;
    }
    size_t off = 0;
    while (off < n) {
        ssize_t w = write(fd, data + off, n - off);
        if (w < 0) {
            char b[256];
            snprintf(b, sizeof(b), "write failed: %s", strerror(errno));
            *err = dup_err(b);
            close(fd);
            return 0;
        }
        off += (size_t)w;
    }
    if (fsync(fd) != 0) {
        char b[256];
        snprintf(b, sizeof(b), "fsync failed: %s", strerror(errno));
        *err = dup_err(b);
        close(fd);
        return 0;
    }
    if (close(fd) != 0) {
        char b[256];
        snprintf(b, sizeof(b), "close failed: %s", strerror(errno));
        *err = dup_err(b);
        return 0;
    }
    return 1;
}

int main(void) {
    // Create temp dir
    char tmpl[] = "./safe_cat_cXXXXXX";
    char *dir = mkdtemp(tmpl);
    if (!dir) {
        fprintf(stderr, "Failed to create temp dir: %s\n", strerror(errno));
        return 1;
    }

    // Create subdir
    char subdir[PATH_MAX];
    snprintf(subdir, sizeof(subdir), "%s/sub", dir);
    if (mkdir(subdir, 0700) != 0) {
        fprintf(stderr, "Failed to create subdir: %s\n", strerror(errno));
        return 1;
    }

    // Prepare files
    char *err = NULL;
    char path[PATH_MAX];

    snprintf(path, sizeof(path), "%s/a.txt", dir);
    write_file(path, "Hello A\n", 8, &err);

    snprintf(path, sizeof(path), "%s/b.txt", dir);
    write_file(path, "Line1\nLine2\n", 12, &err);

    snprintf(path, sizeof(path), "%s/sub/c.txt", dir);
    write_file(path, "Subdir content\n", 16, &err);

    snprintf(path, sizeof(path), "%s/d with spaces.txt", dir);
    write_file(path, "Spaces in name\n", 15, &err);

    snprintf(path, sizeof(path), "%s/e.bin", dir);
    const char bin[] = {'B','i','n','\0','D','a','t','a','\n'};
    write_file(path, bin, sizeof(bin), &err);

    // 5 test cases
    const char *tests[5] = {
        "a.txt",
        "b.txt",
        "sub/c.txt",
        "d with spaces.txt",
        "e.bin"
    };

    for (int i = 0; i < 5; ++i) {
        CatResult r = display_file_via_command(dir, tests[i]);
        printf("Test %d (%s): ", i + 1, tests[i]);
        if (r.ok) {
            printf("[OK]\n");
            printf("Content (%zu bytes):\n", r.out_len);
            fwrite(r.output, 1, r.out_len, stdout);
            printf("\n");
        } else {
            printf("[ERR] %s\n", r.error ? r.error : "unknown error");
        }
        free_result(&r);
    }

    // Negative traversal test (not part of 5)
    CatResult neg = display_file_via_command(dir, "../should_not_work.txt");
    if (neg.ok) {
        printf("Negative traversal test: unexpected OK\n");
    } else {
        printf("Negative traversal test blocked: %s\n", neg.error ? neg.error : "unknown");
    }
    free_result(&neg);

    return 0;
}