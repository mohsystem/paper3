#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <stdbool.h>
#include <dirent.h>

// Safe string join for paths
static int join_paths(char *out, size_t outsz, const char *a, const char *b) {
    if (!a || !b || !out) return -1;
    size_t la = strlen(a);
    size_t lb = strlen(b);
    int needs_slash = (la > 0 && a[la-1] != '/');
    size_t total = la + (needs_slash ? 1 : 0) + lb + 1;
    if (total > outsz) return -1;
    strcpy(out, a);
    if (needs_slash) strcat(out, "/");
    strcat(out, b);
    return 0;
}

// Check if path starts with base as directory prefix
static bool is_within_base(const char *base, const char *path) {
    size_t lb = strlen(base);
    if (strncmp(base, path, lb) != 0) return false;
    // Ensure boundary: either exact match or next char is '/'
    if (path[lb] == '\0') return true;
    if (base[lb-1] == '/') return true;
    return path[lb] == '/';
}

// Step 1: Problem understanding and function definition
// Securely copy a file from a specified base directory to a securely created temporary file.
// Returns a malloc'ed path string to temp file on success, or NULL on failure (caller must free).
char* copy_file_to_temp(const char* base_dir, const char* relative_file) {
    // Step 2: Security requirements - validate inputs
    if (!base_dir || !relative_file) {
        fprintf(stderr, "Error: invalid parameters.\n");
        return NULL;
    }

    char base_real[PATH_MAX];
    if (!realpath(base_dir, base_real)) {
        fprintf(stderr, "Error: base directory not found or not a directory.\n");
        return NULL;
    }

    // Construct candidate path
    char candidate[PATH_MAX];
    if (join_paths(candidate, sizeof(candidate), base_real, relative_file) != 0) {
        fprintf(stderr, "Error: path too long.\n");
        return NULL;
    }

    // Resolve file real path
    char file_real[PATH_MAX];
    if (!realpath(candidate, file_real)) {
        fprintf(stderr, "Error: source file not found.\n");
        return NULL;
    }

    // Ensure within base directory
    if (!is_within_base(base_real, file_real)) {
        fprintf(stderr, "Error: access outside base directory is not allowed.\n");
        return NULL;
    }

    // Ensure it's a regular file and avoid symlink traversal at open with O_NOFOLLOW if available
    struct stat st;
    if (lstat(file_real, &st) != 0 || !S_ISREG(st.st_mode)) {
        fprintf(stderr, "Error: specified path is not a regular file.\n");
        return NULL;
    }

    int in_fd = open(file_real, O_RDONLY
#ifdef O_NOFOLLOW
        | O_NOFOLLOW
#endif
    );
    if (in_fd < 0) {
        fprintf(stderr, "Error: failed to open source file.\n");
        return NULL;
    }

    // Step 3: Secure coding generation - create secure temp file
    char tmpl[PATH_MAX];
    const char* tmpdir = getenv("TMPDIR");
    if (!tmpdir || tmpdir[0] == '\0') tmpdir = "/tmp";
    if (snprintf(tmpl, sizeof(tmpl), "%s/%s", tmpdir, "Task127_XXXXXX") >= (int)sizeof(tmpl)) {
        fprintf(stderr, "Error: temp path too long.\n");
        close(in_fd);
        return NULL;
    }

    int out_fd = mkstemp(tmpl);
    if (out_fd < 0) {
        fprintf(stderr, "Error: failed to create temporary file.\n");
        close(in_fd);
        return NULL;
    }
    // Restrictive permissions
    (void)fchmod(out_fd, S_IRUSR | S_IWUSR);

    // Copy loop
    char buffer[8192];
    ssize_t r;
    while ((r = read(in_fd, buffer, sizeof(buffer))) > 0) {
        ssize_t off = 0;
        while (off < r) {
            ssize_t w = write(out_fd, buffer + off, (size_t)(r - off));
            if (w < 0) {
                fprintf(stderr, "Error: write failure.\n");
                close(in_fd);
                close(out_fd);
                unlink(tmpl);
                return NULL;
            }
            off += w;
        }
    }
    if (r < 0) {
        fprintf(stderr, "Error: read failure.\n");
        close(in_fd);
        close(out_fd);
        unlink(tmpl);
        return NULL;
    }

    close(in_fd);
    close(out_fd);

    // Step 4: Code review - checks in place; returning allocated path
    char* result = (char*)malloc(strlen(tmpl) + 1);
    if (!result) {
        fprintf(stderr, "Error: memory allocation failure.\n");
        unlink(tmpl);
        return NULL;
    }
    strcpy(result, tmpl);
    return result;
}

// Helper to write a test file
static int write_file(const char* dir, const char* rel, const char* content) {
    char path[PATH_MAX];
    if (join_paths(path, sizeof(path), dir, rel) != 0) return -1;

    // Ensure parent directories exist (simple; assumes single-level for tests)
    // For deeper paths, one would iteratively mkdir -p; here tests are simple.
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (fd < 0) return -1;
    if (content) {
        ssize_t len = (ssize_t)strlen(content);
        if (len > 0) {
            if (write(fd, content, (size_t)len) != len) {
                close(fd);
                return -1;
            }
        }
    }
    close(fd);
    return 0;
}

int main(void) {
    // Step 5: Secure code output - run 5 test cases
    char basedir_template[] = "/tmp/Task127_test_c_XXXXXX";
    char* base = mkdtemp(basedir_template);
    if (!base) {
        fprintf(stderr, "Error: failed to create base test directory.\n");
        return 1;
    }

    // Create test files and directories
    if (write_file(base, "file1.txt", "Hello, secure world!") != 0) {
        fprintf(stderr, "Error: failed to create test file.\n");
        return 1;
    }
    if (write_file(base, "empty.txt", "") != 0) {
        fprintf(stderr, "Error: failed to create empty test file.\n");
        return 1;
    }
    // Create subdir
    char subdir[PATH_MAX];
    if (snprintf(subdir, sizeof(subdir), "%s/%s", base, "subdir") >= (int)sizeof(subdir)) return 1;
    mkdir(subdir, 0700);
    // Nested file (not used directly)
    char nested[PATH_MAX];
    if (snprintf(nested, sizeof(nested), "%s/%s", subdir, "nested.txt") >= (int)sizeof(nested)) return 1;
    int nfd = open(nested, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (nfd >= 0) { const char* msg = "Nested content"; write(nfd, msg, strlen(msg)); close(nfd); }

    char* r1 = copy_file_to_temp(base, "file1.txt");
    printf("Test1: %s\n", r1 ? r1 : "(null)");

    char* r2 = copy_file_to_temp(base, "missing.txt");
    printf("Test2: %s\n", r2 ? r2 : "(null)");

    char* r3 = copy_file_to_temp(base, "../file1.txt");
    printf("Test3: %s\n", r3 ? r3 : "(null)");

    char* r4 = copy_file_to_temp(base, "subdir");
    printf("Test4: %s\n", r4 ? r4 : "(null)");

    char* r5 = copy_file_to_temp(base, "empty.txt");
    printf("Test5: %s\n", r5 ? r5 : "(null)");

    if (r1) { free(r1); }
    if (r2) { free(r2); }
    if (r3) { free(r3); }
    if (r4) { free(r4); }
    if (r5) { free(r5); }

    return 0;
}