#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif

#define TAR_BLOCK 512
#define MAX_ENTRY_SIZE (100 * 1024 * 1024) // 100MB

static int read_fully(int fd, void* buf, size_t n) {
    size_t off = 0;
    while (off < n) {
        ssize_t r = read(fd, (char*)buf + off, n - off);
        if (r < 0) return -1;
        if (r == 0) return (int)off; // EOF
        off += (size_t)r;
    }
    return (int)off;
}

static int skip_fully(int fd, size_t n) {
    char buf[8192];
    size_t rem = n;
    while (rem > 0) {
        size_t to = rem < sizeof(buf) ? rem : sizeof(buf);
        ssize_t r = read(fd, buf, to);
        if (r <= 0) return -1;
        rem -= (size_t)r;
    }
    return 0;
}

static int copy_limited(int in_fd, int out_fd, size_t n) {
    char buf[32768];
    size_t rem = n;
    while (rem > 0) {
        size_t to = rem < sizeof(buf) ? rem : sizeof(buf);
        ssize_t r = read(in_fd, buf, to);
        if (r <= 0) return -1;
        size_t off = 0;
        while (off < (size_t)r) {
            ssize_t w = write(out_fd, buf + off, (size_t)r - off);
            if (w <= 0) return -1;
            off += (size_t)w;
        }
        rem -= (size_t)r;
    }
    return 0;
}

static int is_all_zero(const unsigned char* b, size_t n) {
    for (size_t i = 0; i < n; ++i) if (b[i] != 0) return 0;
    return 1;
}

static long long parse_octal(const unsigned char* p, size_t off, size_t len) {
    long long val = 0;
    size_t i = off;
    while (i < off + len && (p[i] == 0 || p[i] == ' ')) i++;
    for (; i < off + len; ++i) {
        unsigned char c = p[i];
        if (c == 0 || c == ' ') break;
        if (c < '0' || c > '7') return -1;
        val = (val << 3) + (c - '0');
        if (val < 0) return -1;
    }
    return val;
}

static int valid_checksum(const unsigned char* h) {
    long long stored = parse_octal(h, 148, 8);
    if (stored < 0) return 0;
    long long sum = 0;
    for (int i = 0; i < 512; ++i) {
        if (i >= 148 && i < 156) sum += 32;
        else sum += (unsigned char)h[i];
    }
    return sum == stored;
}

static void extract_string(const unsigned char* b, size_t off, size_t len, char* out, size_t outsz) {
    size_t i = off;
    size_t end = off + len;
    size_t k = 0;
    while (i < end && b[i] != 0 && k + 1 < outsz) {
        out[k++] = (char)b[i++];
    }
    out[k] = '\0';
}

static int is_safe_rel_path(const char* p) {
    if (!p || !*p) return 0;
    if (p[0] == '/') return 0;
    if (strstr(p, "..") != NULL) return 0;
    if (strchr(p, '\\') != NULL) return 0;
    if (strlen(p) >= 2 && p[1] == ':') return 0;
    return 1;
}

static int mkdirs_at(int basefd, const char* rel) {
    if (!rel || !*rel) return 1;
    char* path = strdup(rel);
    if (!path) return 0;
    int cur = dup(basefd);
    if (cur < 0) { free(path); return 0; }
    char* saveptr = NULL;
    char* token = strtok_r(path, "/", &saveptr);
    while (token) {
        int dfd = openat(cur, token, O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
        if (dfd >= 0) {
            close(cur);
            cur = dfd;
        } else {
            if (errno != ENOENT) { close(cur); free(path); return 0; }
            if (mkdirat(cur, token, 0700) < 0) {
                if (errno != EEXIST) { close(cur); free(path); return 0; }
            }
            dfd = openat(cur, token, O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
            if (dfd < 0) { close(cur); free(path); return 0; }
            close(cur);
            cur = dfd;
        }
        token = strtok_r(NULL, "/", &saveptr);
    }
    close(cur);
    free(path);
    return 1;
}

static int extract_tar(const char* archive_path, const char* dest_dir) {
    if (!archive_path || !dest_dir) {
        fprintf(stderr, "Invalid input\n");
        return 0;
    }
    if (strlen(archive_path) > 4096 || strlen(dest_dir) > 4096) {
        fprintf(stderr, "Path too long\n");
        return 0;
    }
    if (mkdir(dest_dir, 0700) < 0) {
        if (errno != EEXIST) {
            fprintf(stderr, "Failed to prepare destination\n");
            return 0;
        }
    }
    int destfd = open(dest_dir, O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
    if (destfd < 0) {
        fprintf(stderr, "Failed to open destination\n");
        return 0;
    }
    int afd = open(archive_path, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (afd < 0) {
        fprintf(stderr, "Failed to open archive\n");
        close(destfd);
        return 0;
    }

    unsigned char header[TAR_BLOCK];
    int ok = 1;
    int sawZero = 0;

    for (;;) {
        int r = read_fully(afd, header, TAR_BLOCK);
        if (r <= 0) { ok = 0; break; }
        if (r != TAR_BLOCK) { ok = 0; break; }

        if (is_all_zero(header, TAR_BLOCK)) {
            if (sawZero) { ok = 1; break; }
            sawZero = 1;
            continue;
        } else {
            sawZero = 0;
        }

        if (!valid_checksum(header)) {
            fprintf(stderr, "Invalid header checksum\n");
            ok = 0; break;
        }

        char name[101], prefix[156];
        extract_string(header, 0, 100, name, sizeof(name));
        extract_string(header, 345, 155, prefix, sizeof(prefix));
        char rel[312];
        if (prefix[0]) {
            snprintf(rel, sizeof(rel), "%s/%s", prefix, name);
        } else {
            snprintf(rel, sizeof(rel), "%s", name);
        }
        char typeflag = (char)header[156];
        long long sizeLL = parse_octal(header, 124, 12);
        if (sizeLL < 0 || sizeLL > MAX_ENTRY_SIZE) {
            fprintf(stderr, "Invalid size\n");
            ok = 0; break;
        }
        size_t size = (size_t)sizeLL;

        if (!is_safe_rel_path(rel)) {
            fprintf(stderr, "Unsafe path\n");
            ok = 0; break;
        }

        // split parent and base
        char* lastslash = strrchr(rel, '/');
        char parent[312];
        char base[156];
        if (lastslash) {
            size_t plen = (size_t)(lastslash - rel);
            if (plen >= sizeof(parent)) { ok = 0; break; }
            memcpy(parent, rel, plen);
            parent[plen] = '\0';
            snprintf(base, sizeof(base), "%s", lastslash + 1);
        } else {
            parent[0] = '\0';
            snprintf(base, sizeof(base), "%s", rel);
        }

        if (typeflag == '5') {
            if (!mkdirs_at(destfd, rel)) {
                fprintf(stderr, "Failed to create directory\n");
                ok = 0; break;
            }
            size_t pad = (TAR_BLOCK - (size % TAR_BLOCK)) % TAR_BLOCK;
            if (skip_fully(afd, size + pad) != 0) { ok = 0; break; }
        } else if (typeflag == '0' || typeflag == 0) {
            if (parent[0]) {
                if (!mkdirs_at(destfd, parent)) {
                    fprintf(stderr, "Failed to create parent dirs\n");
                    ok = 0; break;
                }
            }
            int pfd = destfd;
            if (parent[0]) {
                pfd = openat(destfd, parent, O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
                if (pfd < 0) { ok = 0; break; }
            }
            int ofd = openat(pfd, base, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW, 0600);
            if (parent[0]) close(pfd);
            if (ofd < 0) { fprintf(stderr, "Create output failed\n"); ok = 0; break; }
            if (copy_limited(afd, ofd, size) != 0) { close(ofd); ok = 0; break; }
            fsync(ofd);
            close(ofd);
            size_t pad = (TAR_BLOCK - (size % TAR_BLOCK)) % TAR_BLOCK;
            if (pad > 0) {
                if (skip_fully(afd, pad) != 0) { ok = 0; break; }
            }
        } else {
            // skip unsupported
            if (size > 0) {
                if (skip_fully(afd, size) != 0) { ok = 0; break; }
                size_t pad = (TAR_BLOCK - (size % TAR_BLOCK)) % TAR_BLOCK;
                if (pad > 0) {
                    if (skip_fully(afd, pad) != 0) { ok = 0; break; }
                }
            }
        }
    }

    fsync(destfd);
    close(afd);
    close(destfd);
    return ok ? 1 : 0;
}

int main(void) {
    printf("Test1: %s\n", extract_tar("archive.zip", "/tmp/unpack") ? "true" : "false");
    printf("Test2: %s\n", extract_tar("nonexistent.tar", "/tmp/unpack") ? "true" : "false");
    printf("Test3: %s\n", extract_tar("", "/tmp/unpack") ? "true" : "false");
    printf("Test4: %s\n", extract_tar("archive.zip", "/tmp/unpack/sub/..") ? "true" : "false");
    printf("Test5: %s\n", extract_tar("archive.zip", "/tmp/unpack") ? "true" : "false");
    return 0;
}