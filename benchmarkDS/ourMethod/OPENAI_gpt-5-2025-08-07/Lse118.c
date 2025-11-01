#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

static int is_valid_filename(const char* name) {
    if (name == NULL) return 0;
    size_t len = strlen(name);
    if (len == 0 || len > 255) return 0;
    if ((len == 1 && name[0] == '.') || (len == 2 && name[0] == '.' && name[1] == '.')) return 0;
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)name[i];
        if (c == '/' || c == '\\') return 0;
        if (!(isalnum(c) || c == '.' || c == '_' || c == '-')) return 0;
    }
    return 1;
}

static int read_urandom_uint32(uint32_t* out) {
    if (out == NULL) return -1;
    int fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (fd < 0) return -1;
    struct stat st;
    if (fstat(fd, &st) != 0 || !S_ISCHR(st.st_mode)) {
        close(fd);
        return -1;
    }
    ssize_t got = 0;
    unsigned char* p = (unsigned char*)out;
    ssize_t need = (ssize_t)sizeof(uint32_t);
    while (got < need) {
        ssize_t r = read(fd, p + got, need - got);
        if (r < 0) {
            close(fd);
            return -1;
        }
        got += r;
    }
    close(fd);
    return 0;
}

static char* generate_and_write(const char* base_dir, const char* filename) {
    if (base_dir == NULL || filename == NULL) {
        return NULL;
    }

    if (!is_valid_filename(filename)) {
        return NULL;
    }

    // Open base directory
    int dirfd = open(base_dir, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (dirfd < 0) {
        return NULL;
    }
    struct stat ds;
    if (fstat(dirfd, &ds) != 0 || !S_ISDIR(ds.st_mode)) {
        close(dirfd);
        return NULL;
    }

    // Generate 3 random floats in [0,1)
    uint32_t r1 = 0, r2 = 0, r3 = 0;
    if (read_urandom_uint32(&r1) != 0 ||
        read_urandom_uint32(&r2) != 0 ||
        read_urandom_uint32(&r3) != 0) {
        close(dirfd);
        return NULL;
    }
    double f1 = (double)r1 / (double)UINT32_MAX;
    double f2 = (double)r2 / (double)UINT32_MAX;
    double f3 = (double)r3 / (double)UINT32_MAX;

    char s1[64], s2[64], s3[64];
    if (snprintf(s1, sizeof(s1), "%.9g", f1) < 0 ||
        snprintf(s2, sizeof(s2), "%.9g", f2) < 0 ||
        snprintf(s3, sizeof(s3), "%.9g", f3) < 0) {
        close(dirfd);
        return NULL;
    }

    size_t len = strlen(s1) + strlen(s2) + strlen(s3);
    char* content = (char*)malloc(len + 1);
    if (!content) {
        close(dirfd);
        return NULL;
    }
    content[0] = '\0';
    strncat(content, s1, len + 1 - strlen(content) - 1);
    strncat(content, s2, len + 1 - strlen(content) - 1);
    strncat(content, s3, len + 1 - strlen(content) - 1);

    // Create temp file name
    char tmpname[128];
    int tmpfd = -1;
    for (int attempt = 0; attempt < 20; ++attempt) {
        uint32_t rnd = 0;
        if (read_urandom_uint32(&rnd) != 0) rnd = (uint32_t)getpid() ^ (uint32_t)attempt;
        if (snprintf(tmpname, sizeof(tmpname), "tmp-%d-%u.tmp", (int)getpid(), rnd) < 0) {
            free(content);
            close(dirfd);
            return NULL;
        }
        tmpfd = openat(dirfd, tmpname, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW, 0600);
        if (tmpfd >= 0) break;
        if (errno != EEXIST) {
            free(content);
            close(dirfd);
            return NULL;
        }
    }
    if (tmpfd < 0) {
        free(content);
        close(dirfd);
        return NULL;
    }

    // Write content
    size_t to_write = strlen(content);
    size_t written = 0;
    while (written < to_write) {
        ssize_t w = write(tmpfd, content + written, to_write - written);
        if (w < 0) {
            int e = errno;
            (void)close(tmpfd);
            (void)unlinkat(dirfd, tmpname, 0);
            free(content);
            close(dirfd);
            errno = e;
            return NULL;
        }
        written += (size_t)w;
    }

    // fsync and close
    (void)fsync(tmpfd);
    (void)close(tmpfd);

    // Rename to target
    if (renameat(dirfd, tmpname, dirfd, filename) != 0) {
        (void)unlinkat(dirfd, tmpname, 0);
        free(content);
        close(dirfd);
        return NULL;
    }

    // fsync directory
    (void)fsync(dirfd);
    close(dirfd);

    return content; // caller must free
}

int main(void) {
    // Create temporary directory
    char tmpl[] = "/tmp/safe-out-XXXXXX";
    char* dirpath = mkdtemp(tmpl);
    if (!dirpath) {
        fprintf(stderr, "Failed to create temp dir\n");
        return 1;
    }
    printf("Base dir: %s\n", dirpath);

    char* r1 = generate_and_write(dirpath, "case1.txt");
    if (r1) { printf("case1 -> %s\n", r1); free(r1); } else { printf("Error\n"); }

    char* r2 = generate_and_write(dirpath, "case2.txt");
    if (r2) { printf("case2 -> %s\n", r2); free(r2); } else { printf("Error\n"); }

    char* r3 = generate_and_write(dirpath, "case3.txt");
    if (r3) { printf("case3 -> %s\n", r3); free(r3); } else { printf("Error\n"); }

    char* r4 = generate_and_write(dirpath, "case4.txt");
    if (r4) { printf("case4 -> %s\n", r4); free(r4); } else { printf("Error\n"); }

    char* r5 = generate_and_write(dirpath, "case5.txt");
    if (r5) { printf("case5 -> %s\n", r5); free(r5); } else { printf("Error\n"); }

    return 0;
}