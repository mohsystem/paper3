#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include <time.h>

#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif

#ifndef AT_SYMLINK_NOFOLLOW
#define AT_SYMLINK_NOFOLLOW 0
#endif

#define MAX_SIZE (8 * 1024 * 1024) // 8 MiB

typedef struct {
    uint64_t bytesRead;
    uint32_t checksum;
    int success;
    const char* message;
} ProcessingResult;

static int is_safe_rel_path(const char* rel) {
    if (rel == NULL || rel[0] == '\0') return 0;
    if (rel[0] == '/' || rel[0] == '\\') return 0;
    for (const char* p = rel; *p; ++p) {
        if (*p == '\0') return 0;
    }
    char tmp[PATH_MAX];
    if (strlen(rel) >= sizeof(tmp)) return 0;
    strcpy(tmp, rel);
    char* saveptr = NULL;
    char* token = strtok_r(tmp, "/", &saveptr);
    while (token) {
        if (strlen(token) == 0) return 0;
        if (strcmp(token, ".") == 0 || strcmp(token, "..") == 0) return 0;
        for (char* q = token; *q; ++q) {
            if (*q == '\\' || *q == '\0') return 0;
        }
        token = strtok_r(NULL, "/", &saveptr);
    }
    return 1;
}

static int ensure_dirs(int dirfd, const char* relpath) {
    char path[PATH_MAX];
    if (strlen(relpath) >= sizeof(path)) return 0;
    strcpy(path, relpath);
    char cur[PATH_MAX] = {0};
    char* saveptr = NULL;
    char* token = strtok_r(path, "/", &saveptr);
    size_t idx = 0;
    size_t total = 0;
    // Count segments
    char* parts[256];
    while (token && idx < 256) {
        parts[idx++] = token;
        token = strtok_r(NULL, "/", &saveptr);
    }
    // create directories for all but last segment
    for (size_t i = 0; i + 1 < idx; ++i) {
        if (i == 0) snprintf(cur, sizeof(cur), "%s", parts[i]);
        else snprintf(cur, sizeof(cur), "%s/%s", cur, parts[i]);
        if (mkdirat(dirfd, cur, 0700) != 0) {
            if (errno == EEXIST) {
                struct stat st;
                if (fstatat(dirfd, cur, &st, AT_SYMLINK_NOFOLLOW) != 0) return 0;
                if (!S_ISDIR(st.st_mode)) return 0;
            } else {
                return 0;
            }
        }
    }
    return 1;
}

static int gen_unique_tmp(int dirfd, char* namebuf, size_t namebuf_sz) {
    for (int attempt = 0; attempt < 100; ++attempt) {
        unsigned int r = (unsigned int)getpid() ^ (unsigned int)time(NULL) ^ (unsigned int)attempt;
        if (snprintf(namebuf, namebuf_sz, ".tmp_%u_%d", r, attempt) >= (int)namebuf_sz) {
            return 0;
        }
        int fd = openat(dirfd, namebuf, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW, 0600);
        if (fd >= 0) {
            close(fd);
            return 1;
        } else if (errno == EEXIST) {
            continue;
        } else {
            return 0;
        }
    }
    return 0;
}

ProcessingResult process_file(const char* baseDir, const char* inputRel, const char* outputRel) {
    if (!is_safe_rel_path(inputRel) || !is_safe_rel_path(outputRel)) {
        ProcessingResult r = {0, 0, 0, "Invalid relative path"};
        return r;
    }

    int bfd = open(baseDir, O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
    if (bfd < 0) {
        ProcessingResult r = {0, 0, 0, "Base dir open failed"};
        return r;
    }

    int ifd = openat(bfd, inputRel, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (ifd < 0) {
        close(bfd);
        ProcessingResult r = {0, 0, 0, "Input open failed"};
        return r;
    }

    struct stat st;
    if (fstat(ifd, &st) != 0) {
        close(ifd);
        close(bfd);
        ProcessingResult r = {0, 0, 0, "fstat failed"};
        return r;
    }
    if (!S_ISREG(st.st_mode)) {
        close(ifd);
        close(bfd);
        ProcessingResult r = {0, 0, 0, "Not a regular file"};
        return r;
    }
    if ((size_t)st.st_size > MAX_SIZE) {
        close(ifd);
        close(bfd);
        ProcessingResult r = {0, 0, 0, "File too large"};
        return r;
    }

    uint64_t bytesRead = 0;
    uint32_t checksum = 0;
    unsigned char buf[8192];
    while (1) {
        ssize_t n = read(ifd, buf, sizeof(buf));
        if (n < 0) {
            close(ifd);
            close(bfd);
            ProcessingResult r = {0, 0, 0, "Read failed"};
            return r;
        }
        if (n == 0) break;
        bytesRead += (uint64_t)n;
        for (ssize_t i = 0; i < n; ++i) {
            checksum = (checksum + buf[i]) & 0xFFFFFFFFu;
        }
    }
    close(ifd);

    if (!ensure_dirs(bfd, outputRel)) {
        close(bfd);
        ProcessingResult r = {0, 0, 0, "Dir create failed"};
        return r;
    }

    char tmpname[64];
    if (!gen_unique_tmp(bfd, tmpname, sizeof(tmpname))) {
        close(bfd);
        ProcessingResult r = {0, 0, 0, "Temp create failed"};
        return r;
    }

    int tfd = openat(bfd, tmpname, O_WRONLY | O_CLOEXEC | O_NOFOLLOW);
    if (tfd < 0) {
        unlinkat(bfd, tmpname, 0);
        close(bfd);
        ProcessingResult r = {0, 0, 0, "Temp open failed"};
        return r;
    }

    char outbuf[128];
    int len = snprintf(outbuf, sizeof(outbuf), "%llu,%u\n",
                       (unsigned long long)bytesRead, (unsigned int)checksum);
    if (len < 0 || len >= (int)sizeof(outbuf)) {
        close(tfd);
        unlinkat(bfd, tmpname, 0);
        close(bfd);
        ProcessingResult r = {0, 0, 0, "Format error"};
        return r;
    }
    int off = 0;
    while (off < len) {
        ssize_t w = write(tfd, outbuf + off, (size_t)(len - off));
        if (w <= 0) {
            close(tfd);
            unlinkat(bfd, tmpname, 0);
            close(bfd);
            ProcessingResult r = {0, 0, 0, "Write failed"};
            return r;
        }
        off += (int)w;
    }
    if (fsync(tfd) != 0) {
        close(tfd);
        unlinkat(bfd, tmpname, 0);
        close(bfd);
        ProcessingResult r = {0, 0, 0, "fsync failed"};
        return r;
    }
    close(tfd);

    if (renameat(bfd, tmpname, bfd, outputRel) != 0) {
        // try replacement
        unlinkat(bfd, outputRel, 0);
        if (renameat(bfd, tmpname, bfd, outputRel) != 0) {
            unlinkat(bfd, tmpname, 0);
            close(bfd);
            ProcessingResult r = {0, 0, 0, "Rename failed"};
            return r;
        }
    }

    fsync(bfd);
    close(bfd);

    ProcessingResult ok = {bytesRead, checksum, 1, "OK"};
    return ok;
}

static int write_file(int dirfd, const char* rel, const unsigned char* data, size_t len) {
    if (!ensure_dirs(dirfd, rel)) return 0;
    int fd = openat(dirfd, rel, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC | O_NOFOLLOW, 0600);
    if (fd < 0) return 0;
    size_t off = 0;
    while (off < len) {
        ssize_t w = write(fd, data + off, len - off);
        if (w <= 0) { close(fd); return 0; }
        off += (size_t)w;
    }
    fsync(fd);
    close(fd);
    return 1;
}

int main(void) {
    char templ[] = "/tmp/secure_io_base_c_XXXXXX";
    char* basedir = mkdtemp(templ);
    if (!basedir) {
        fprintf(stderr, "mkdtemp failed\n");
        return 1;
    }
    printf("Base dir: %s\n", basedir);
    int bfd = open(basedir, O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
    if (bfd < 0) {
        fprintf(stderr, "open base failed\n");
        return 1;
    }

    // Prepare test inputs
    write_file(bfd, "empty.txt", (const unsigned char*)"", 0);
    const unsigned char hello[] = "Hello, World!";
    write_file(bfd, "hello.txt", hello, sizeof(hello) - 1);
    unsigned char zeros[1024]; memset(zeros, 0, sizeof(zeros));
    write_file(bfd, "zeros.bin", zeros, sizeof(zeros));
    unsigned char repeated[3000];
    for (int i = 0; i < 1000; ++i) { repeated[i*3] = 'a'; repeated[i*3+1] = 'b'; repeated[i*3+2] = 'c'; }
    write_file(bfd, "repeated.txt", repeated, sizeof(repeated));
    close(bfd);

    ProcessingResult r1 = process_file(basedir, "empty.txt", "out/out1.txt");
    printf("Test1: bytes=%llu checksum=%u success=%d msg=%s\n",
           (unsigned long long)r1.bytesRead, r1.checksum, r1.success, r1.message);

    ProcessingResult r2 = process_file(basedir, "hello.txt", "out/out2.txt");
    printf("Test2: bytes=%llu checksum=%u success=%d msg=%s\n",
           (unsigned long long)r2.bytesRead, r2.checksum, r2.success, r2.message);

    ProcessingResult r3 = process_file(basedir, "zeros.bin", "out/out3.txt");
    printf("Test3: bytes=%llu checksum=%u success=%d msg=%s\n",
           (unsigned long long)r3.bytesRead, r3.checksum, r3.success, r3.message);

    ProcessingResult r4 = process_file(basedir, "repeated.txt", "out/out4.txt");
    printf("Test4: bytes=%llu checksum=%u success=%d msg=%s\n",
           (unsigned long long)r4.bytesRead, r4.checksum, r4.success, r4.message);

    ProcessingResult r5 = process_file(basedir, "../evil.txt", "out/out5.txt");
    printf("Test5: bytes=%llu checksum=%u success=%d msg=%s\n",
           (unsigned long long)r5.bytesRead, r5.checksum, r5.success, r5.message);

    return 0;
}