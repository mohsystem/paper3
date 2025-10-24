#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

// Split a relative path into components; reject absolute paths, '.', '..', empty components, backslashes
static int split_relative_path(const char *relPath, char components[][256], size_t *count) {
    if (!relPath || !count) return 0;
    size_t len = strnlen(relPath, 4097);
    if (len == 0 || len > 4096) return 0;
    if (relPath[0] == '/') return 0;
    for (size_t i = 0; i < len; ++i) {
        if (relPath[i] == '\\' || relPath[i] == '\0') return 0;
    }

    size_t idx = 0;
    const char *start = relPath;
    const char *p = relPath;
    while (1) {
        if (*p == '/' || *p == '\0') {
            size_t seglen = (size_t)(p - start);
            if (seglen == 0) return 0; // empty component
            if (seglen >= sizeof(components[0])) return 0; // too long
            if (idx >= *count) return 0; // too many components
            // copy segment
            char buf[256];
            memcpy(buf, start, seglen);
            buf[seglen] = '\0';
            if (strcmp(buf, ".") == 0 || strcmp(buf, "..") == 0) return 0;
            strncpy(components[idx], buf, sizeof(components[idx]) - 1);
            components[idx][sizeof(components[idx]) - 1] = '\0';
            idx++;
            if (*p == '\0') break;
            start = p + 1;
        }
        if (*p == '\0') break;
        p++;
    }
    *count = idx;
    return idx > 0;
}

static int open_dir_component_nofollow(int parent_fd, const char *name) {
    return openat(parent_fd, name, O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
}

static int verify_final_regular_file(int dirfd, const char *name) {
    int fd = openat(dirfd, name, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (fd == -1) {
        return -1; // ELOOP, ENOENT, etc.
    }
    struct stat st;
    if (fstat(fd, &st) != 0) {
        int e = errno;
        close(fd);
        errno = e;
        return -1;
    }
    close(fd);
    if (!S_ISREG(st.st_mode)) {
        errno = EISDIR;
        return -1;
    }
    return 0;
}

// Secure delete function: deletes relPath under baseDir
// Returns 0 on success, -1 on error with errno set.
int delete_file_secure_c(const char *baseDir, const char *relPath) {
    if (!baseDir || !relPath) { errno = EINVAL; return -1; }
    size_t baseLen = strnlen(baseDir, 4097);
    if (baseLen == 0 || baseLen > 4096) { errno = EINVAL; return -1; }

    char parts[128][256];
    size_t count = 128;
    if (!split_relative_path(relPath, parts, &count)) {
        errno = EINVAL;
        return -1;
    }

    int dirfd = open(baseDir, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (dirfd == -1) return -1;

    for (size_t i = 0; i + 1 < count; ++i) {
        int nextfd = open_dir_component_nofollow(dirfd, parts[i]);
        if (nextfd == -1) {
            int e = errno;
            close(dirfd);
            errno = e;
            return -1;
        }
        close(dirfd);
        dirfd = nextfd;
    }

    if (verify_final_regular_file(dirfd, parts[count - 1]) != 0) {
        int e = errno;
        close(dirfd);
        errno = e;
        return -1;
    }

    if (unlinkat(dirfd, parts[count - 1], 0) != 0) {
        int e = errno;
        close(dirfd);
        errno = e;
        return -1;
    }

    close(dirfd);
    return 0;
}

// Helpers for tests
static int ensure_dir(const char *path) {
    if (mkdir(path, 0700) == 0) return 1;
    if (errno == EEXIST) {
        struct stat st;
        if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) return 1;
    }
    return 0;
}

static int ensure_file(const char *path) {
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0600);
    if (fd == -1) return 0;
    const char *msg = "x";
    ssize_t w = write(fd, msg, 1);
    int ok = (w == 1);
    close(fd);
    return ok;
}

static int ensure_symlink(const char *target, const char *linkpath) {
    unlink(linkpath);
    return symlink(target, linkpath) == 0;
}

static void print_result(const char *name, int rc) {
    if (rc == 0) {
        printf("%s: success\n", name);
    } else {
        printf("%s: failure (%s)\n", name, strerror(errno));
    }
}

int main(int argc, char *argv[]) {
    if (argc >= 3) {
        const char *base = argv[1];
        const char *rel = argv[2];
        int rc = delete_file_secure_c(base, rel);
        if (rc == 0) {
            printf("Deleted: %s\n", rel);
            return 0;
        } else {
            fprintf(stderr, "Error: %s\n", strerror(errno));
            return 1;
        }
    }

    // Self-tests (5 cases)
    const char *base = "sandbox_c";
    ensure_dir(base);

    // Test 1
    ensure_file("sandbox_c/a.txt");
    print_result("Test1 delete a.txt", delete_file_secure_c(base, "a.txt"));

    // Test 2
    ensure_dir("sandbox_c/dir1");
    ensure_file("sandbox_c/dir1/b.txt");
    print_result("Test2 delete dir1/b.txt", delete_file_secure_c(base, "dir1/b.txt"));

    // Test 3
    ensure_file("sandbox_c/a.txt");
    ensure_symlink("a.txt", "sandbox_c/link_to_a");
    print_result("Test3 reject symlink link_to_a", delete_file_secure_c(base, "link_to_a"));

    // Test 4
    print_result("Test4 reject '..' traversal", delete_file_secure_c(base, "../evil.txt"));

    // Test 5
    ensure_dir("sandbox_c/dir1/subdir");
    print_result("Test5 reject directory", delete_file_secure_c(base, "dir1/subdir"));

    return 0;
}