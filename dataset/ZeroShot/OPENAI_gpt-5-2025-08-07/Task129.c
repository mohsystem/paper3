#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_FILE_SIZE 1048576 /* 1 MB */

static int is_valid_filename(const char* name) {
    if (!name) return 0;
    size_t len = strlen(name);
    if (len == 0 || len > 128) return 0;
    if (name[0] == '.') return 0;
    for (size_t i = 0; i < len; ++i) {
        char c = name[i];
        if (!(isalnum((unsigned char)c) || c == '_' || c == '-' || c == '.')) {
            return 0;
        }
    }
    const char* dot = strrchr(name, '.');
    if (!dot || dot == name || *(dot + 1) == '\0') return 0;
    const char* ext = dot + 1;
    char lower[8] = {0};
    size_t elen = strlen(ext);
    if (elen >= sizeof(lower)) return 0;
    for (size_t i = 0; i < elen; ++i) lower[i] = (char)tolower((unsigned char)ext[i]);
    if (strcmp(lower, "txt") == 0 || strcmp(lower, "json") == 0 || strcmp(lower, "csv") == 0) {
        return 1;
    }
    return 0;
}

char* fetch_file(const char* base_dir, const char* filename) {
    if (!base_dir || !filename) {
        errno = EINVAL;
        return NULL;
    }
    if (!is_valid_filename(filename)) {
        errno = EPERM;
        return NULL;
    }

    char base_real[PATH_MAX];
    if (!realpath(base_dir, base_real)) {
        return NULL;
    }

    char path[PATH_MAX];
    int n = snprintf(path, sizeof(path), "%s/%s", base_real, filename);
    if (n < 0 || (size_t)n >= sizeof(path)) {
        errno = ENAMETOOLONG;
        return NULL;
    }

    struct stat st;
    if (lstat(path, &st) != 0) {
        return NULL;
    }
    if (!S_ISREG(st.st_mode)) {
        errno = EPERM;
        return NULL;
    }
#ifdef O_NOFOLLOW
    int fd = open(path, O_RDONLY | O_NOFOLLOW);
#else
    int fd = open(path, O_RDONLY);
#endif
    if (fd < 0) {
        return NULL;
    }

    if (fstat(fd, &st) != 0) {
        close(fd);
        return NULL;
    }
    if (!S_ISREG(st.st_mode)) {
        close(fd);
        errno = EPERM;
        return NULL;
    }
    if (st.st_size < 0 || st.st_size > MAX_FILE_SIZE) {
        close(fd);
        errno = EFBIG;
        return NULL;
    }

    size_t size = (size_t)st.st_size;
    char* buf = (char*)malloc(size + 1);
    if (!buf) {
        close(fd);
        return NULL;
    }

    size_t off = 0;
    while (off < size) {
        ssize_t r = read(fd, buf + off, size - off);
        if (r < 0) {
            free(buf);
            close(fd);
            return NULL;
        }
        if (r == 0) break;
        off += (size_t)r;
    }
    buf[off] = '\0';
    close(fd);
    return buf;
}

static int write_file(const char* path, const char* content, size_t len) {
    FILE* f = fopen(path, "wb");
    if (!f) return -1;
    size_t w = fwrite(content, 1, len, f);
    fclose(f);
    return (w == len) ? 0 : -1;
}

int main(void) {
    char tmpl[] = "/tmp/task129_c_XXXXXX";
    char* base = mkdtemp(tmpl);
    if (!base) {
        printf("Setup failed\n");
        return 1;
    }

    char p1[PATH_MAX], p2[PATH_MAX], p3[PATH_MAX], pBig[PATH_MAX];
    snprintf(p1, sizeof(p1), "%s/%s", base, "a.txt");
    snprintf(p2, sizeof(p2), "%s/%s", base, "data.json");
    snprintf(p3, sizeof(p3), "%s/%s", base, "doc.csv");
    snprintf(pBig, sizeof(pBig), "%s/%s", base, "big.txt");

    write_file(p1, "Hello from C\n", strlen("Hello from C\n"));
    write_file(p2, "{\"k\":\"v\"}\n", strlen("{\"k\":\"v\"}\n"));
    write_file(p3, "c1,c2\n1,2\n", strlen("c1,c2\n1,2\n"));
    {
        char* big = (char*)malloc(MAX_FILE_SIZE + 5);
        if (big) {
            memset(big, 'A', MAX_FILE_SIZE + 5);
            write_file(pBig, big, MAX_FILE_SIZE + 5);
            free(big);
        }
    }

    const char* tests[5] = {
        "a.txt",
        "data.json",
        "../etc/passwd",
        "secret.conf",
        "big.txt"
    };

    for (int i = 0; i < 5; ++i) {
        char* content = fetch_file(base, tests[i]);
        if (content) {
            for (char* p = content; *p; ++p) {
                if (*p == '\n') *p = '\\';
            }
            printf("FETCH OK [%s]: %s\n", tests[i], content);
            free(content);
        } else {
            printf("FETCH FAIL [%s]: %s\n", tests[i], strerror(errno));
        }
    }

    // Cleanup best-effort
    unlink(p1);
    unlink(p2);
    unlink(p3);
    unlink(pBig);
    rmdir(base);
    return 0;
}