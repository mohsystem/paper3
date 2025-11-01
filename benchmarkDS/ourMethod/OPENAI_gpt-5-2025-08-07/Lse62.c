#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif

#define MAX_FILE_SIZE (16 * 1024 * 1024) /* 16 MiB */

typedef struct {
    size_t bytes_read;
    uint32_t checksum;
    int empty;
} Result;

static int is_safe_relative(const char* rel) {
    if (rel == NULL || rel[0] == '\0') return 0;
    if (rel[0] == '/') return 0;
    if (strstr(rel, "\0") != NULL) return 0;
    // reject ".." components
    const char* p = rel;
    while (*p) {
        const char* slash = strchr(p, '/');
        size_t len = slash ? (size_t)(slash - p) : strlen(p);
        if ((len == 2 && strncmp(p, "..", 2) == 0)) return 0;
        p = slash ? slash + 1 : p + len;
    }
    return 1;
}

static int split_dir_file(const char* rel, char** out_dir, char** out_leaf) {
    const char* last = strrchr(rel, '/');
    if (!last) {
        *out_dir = NULL;
        *out_leaf = strdup(rel);
        if (!*out_leaf) return -1;
        return 0;
    }
    size_t dlen = (size_t)(last - rel);
    if (dlen == 0) {
        *out_dir = NULL;
    } else {
        *out_dir = (char*)malloc(dlen + 1);
        if (!*out_dir) return -1;
        memcpy(*out_dir, rel, dlen);
        (*out_dir)[dlen] = '\0';
    }
    *out_leaf = strdup(last + 1);
    if (!*out_leaf) {
        if (*out_dir) free(*out_dir);
        return -1;
    }
    if ((*out_leaf)[0] == '\0' || strcmp(*out_leaf, ".") == 0 || strcmp(*out_leaf, "..") == 0) {
        if (*out_dir) free(*out_dir);
        free(*out_leaf);
        return -1;
    }
    return 0;
}

static int open_base_dirfd(const char* base_dir) {
    int fd = open(base_dir, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    return fd;
}

static int ensure_dir_hierarchy(int base_dirfd, const char* dir_rel) {
    if (dir_rel == NULL || dir_rel[0] == '\0') {
        int dupfd = fcntl(base_dirfd, F_DUPFD_CLOEXEC, 0);
        return dupfd;
    }
    int curfd = fcntl(base_dirfd, F_DUPFD_CLOEXEC, 0);
    if (curfd < 0) return -1;

    const char* p = dir_rel;
    while (*p) {
        const char* slash = strchr(p, '/');
        size_t len = slash ? (size_t)(slash - p) : strlen(p);
        if (len == 0 || (len == 1 && p[0] == '.') || (len == 2 && p[0] == '.' && p[1] == '.')) {
            close(curfd);
            errno = EINVAL;
            return -1;
        }
        char comp[256];
        if (len >= sizeof(comp)) { close(curfd); errno = ENAMETOOLONG; return -1; }
        memcpy(comp, p, len);
        comp[len] = '\0';

        if (mkdirat(curfd, comp, 0700) < 0) {
            if (errno != EEXIST) { int e = errno; close(curfd); errno = e; return -1; }
        }
        int nextfd = openat(curfd, comp, O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
        if (nextfd < 0) { int e = errno; close(curfd); errno = e; return -1; }
        close(curfd);
        curfd = nextfd;

        p = slash ? slash + 1 : p + len;
    }
    return curfd;
}

static int write_atomic_at(int dirfd, const char* leaf, const char* content) {
    char tmpname[512];
    if (strlen(leaf) > 400) { errno = ENAMETOOLONG; return -1; }
    snprintf(tmpname, sizeof(tmpname), "%s.tmp-%ld-%ld", leaf, (long)getpid(), (long)random());

    int fd = openat(dirfd, tmpname, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW, 0600);
    if (fd < 0) return -1;
    size_t len = strlen(content);
    size_t off = 0;
    while (off < len) {
        ssize_t w = write(fd, content + off, len - off);
        if (w < 0) { int e = errno; close(fd); unlinkat(dirfd, tmpname, 0); errno = e; return -1; }
        off += (size_t)w;
    }
    if (fsync(fd) < 0) { int e = errno; close(fd); unlinkat(dirfd, tmpname, 0); errno = e; return -1; }
    if (close(fd) < 0) { int e = errno; unlinkat(dirfd, tmpname, 0); errno = e; return -1; }
    if (renameat(dirfd, tmpname, dirfd, leaf) < 0) { int e = errno; unlinkat(dirfd, tmpname, 0); errno = e; return -1; }
    (void)fsync(dirfd);
    return 0;
}

int process_file(const char* base_dir, const char* input_rel, const char* output_rel, Result* out_res) {
    if (!base_dir || !input_rel || !output_rel || !out_res) return -1;
    if (!is_safe_relative(input_rel) || !is_safe_relative(output_rel)) return -1;

    int basedirfd = open_base_dirfd(base_dir);
    if (basedirfd < 0) return -1;

    int infd = openat(basedirfd, input_rel, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (infd < 0) { int e = errno; close(basedirfd); errno = e; return -1; }

    struct stat st;
    if (fstat(infd, &st) < 0) { int e = errno; close(infd); close(basedirfd); errno = e; return -1; }
    if (!S_ISREG(st.st_mode)) { close(infd); close(basedirfd); errno = EINVAL; return -1; }
    if (st.st_size < 0 || st.st_size > MAX_FILE_SIZE) { close(infd); close(basedirfd); errno = EFBIG; return -1; }

    size_t size = (size_t)st.st_size;
    uint8_t* buf = NULL;
    if (size > 0) {
        buf = (uint8_t*)malloc(size);
        if (!buf) { close(infd); close(basedirfd); errno = ENOMEM; return -1; }
    }

    size_t total = 0;
    while (total < size) {
        ssize_t n = read(infd, buf + total, size - total);
        if (n < 0) { int e = errno; free(buf); close(infd); close(basedirfd); errno = e; return -1; }
        if (n == 0) break;
        total += (size_t)n;
    }
    close(infd);

    uint32_t checksum = 0;
    for (size_t i = 0; i < total; i++) {
        checksum = (uint32_t)(checksum + buf[i]);
    }
    if (buf) free(buf);

    char* outdir = NULL;
    char* outleaf = NULL;
    if (split_dir_file(output_rel, &outdir, &outleaf) != 0) { close(basedirfd); errno = EINVAL; return -1; }

    int outdirfd = ensure_dir_hierarchy(basedirfd, outdir ? outdir : "");
    if (outdirfd < 0) {
        int e = errno;
        free(outdir);
        free(outleaf);
        close(basedirfd);
        errno = e;
        return -1;
    }

    char line[128];
    int m = snprintf(line, sizeof(line), "%zu %u\n", total, checksum);
    if (m < 0 || (size_t)m >= sizeof(line)) {
        free(outdir);
        free(outleaf);
        close(outdirfd);
        close(basedirfd);
        errno = EOVERFLOW;
        return -1;
    }

    if (write_atomic_at(outdirfd, outleaf, line) != 0) {
        int e = errno;
        free(outdir);
        free(outleaf);
        close(outdirfd);
        close(basedirfd);
        errno = e;
        return -1;
    }

    (void)close(outdirfd);
    (void)close(basedirfd);
    out_res->bytes_read = total;
    out_res->checksum = checksum;
    out_res->empty = (checksum == 0);
    free(outdir);
    free(outleaf);
    return 0;
}

static int safe_write_input(const char* base_dir, const char* rel, const uint8_t* data, size_t len) {
    if (!is_safe_relative(rel)) { errno = EINVAL; return -1; }
    int basedirfd = open_base_dirfd(base_dir);
    if (basedirfd < 0) return -1;

    char* dir = NULL;
    char* leaf = NULL;
    if (split_dir_file(rel, &dir, &leaf) != 0) { close(basedirfd); errno = EINVAL; return -1; }
    int dirfd = ensure_dir_hierarchy(basedirfd, dir ? dir : "");
    if (dirfd < 0) { int e = errno; free(dir); free(leaf); close(basedirfd); errno = e; return -1; }

    char tmpname[512];
    if (strlen(leaf) > 400) { free(dir); free(leaf); close(dirfd); close(basedirfd); errno = ENAMETOOLONG; return -1; }
    snprintf(tmpname, sizeof(tmpname), "%s.tmp-%ld-%ld", leaf, (long)getpid(), (long)random());
    int fd = openat(dirfd, tmpname, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW, 0600);
    if (fd < 0) { int e = errno; free(dir); free(leaf); close(dirfd); close(basedirfd); errno = e; return -1; }
    size_t off = 0;
    while (off < len) {
        ssize_t w = write(fd, data + off, len - off);
        if (w < 0) { int e = errno; close(fd); unlinkat(dirfd, tmpname, 0); free(dir); free(leaf); close(dirfd); close(basedirfd); errno = e; return -1; }
        off += (size_t)w;
    }
    fsync(fd);
    close(fd);
    if (renameat(dirfd, tmpname, dirfd, leaf) < 0) { int e = errno; unlinkat(dirfd, tmpname, 0); free(dir); free(leaf); close(dirfd); close(basedirfd); errno = e; return -1; }
    fsync(dirfd);

    free(dir);
    free(leaf);
    close(dirfd);
    close(basedirfd);
    return 0;
}

int main(void) {
    const char* base = "sandbox_c";
    mkdir(base, 0700);

    // Prepare 5 input files
    safe_write_input(base, "inputs/empty.bin", (const uint8_t*)"", 0);
    const uint8_t hello[] = { 'h','e','l','l','o' };
    safe_write_input(base, "inputs/hello.txt", hello, sizeof(hello));
    uint8_t kb[1024]; memset(kb, 'A', sizeof(kb));
    safe_write_input(base, "inputs/1kb.bin", kb, sizeof(kb));
    uint8_t seq[256]; for (int i = 0; i < 256; i++) seq[i] = (uint8_t)i;
    safe_write_input(base, "inputs/seq.bin", seq, sizeof(seq));
    uint8_t zeros[10000]; memset(zeros, '0', sizeof(zeros));
    safe_write_input(base, "inputs/zeros.txt", zeros, sizeof(zeros));

    // 5 test cases
    Result r;
    if (process_file(base, "inputs/empty.bin", "outputs/empty.out", &r) == 0)
        printf("T1: bytes=%zu checksum=%u empty=%s\n", r.bytes_read, r.checksum, r.empty ? "true" : "false");
    else
        fprintf(stderr, "T1 error: %s\n", strerror(errno));

    if (process_file(base, "inputs/hello.txt", "outputs/hello.out", &r) == 0)
        printf("T2: bytes=%zu checksum=%u empty=%s\n", r.bytes_read, r.checksum, r.empty ? "true" : "false");
    else
        fprintf(stderr, "T2 error: %s\n", strerror(errno));

    if (process_file(base, "inputs/1kb.bin", "outputs/1kb.out", &r) == 0)
        printf("T3: bytes=%zu checksum=%u empty=%s\n", r.bytes_read, r.checksum, r.empty ? "true" : "false");
    else
        fprintf(stderr, "T3 error: %s\n", strerror(errno));

    if (process_file(base, "inputs/seq.bin", "outputs/seq.out", &r) == 0)
        printf("T4: bytes=%zu checksum=%u empty=%s\n", r.bytes_read, r.checksum, r.empty ? "true" : "false");
    else
        fprintf(stderr, "T4 error: %s\n", strerror(errno));

    if (process_file(base, "inputs/zeros.txt", "outputs/zeros.out", &r) == 0)
        printf("T5: bytes=%zu checksum=%u empty=%s\n", r.bytes_read, r.checksum, r.empty ? "true" : "false");
    else
        fprintf(stderr, "T5 error: %s\n", strerror(errno));

    return 0;
}