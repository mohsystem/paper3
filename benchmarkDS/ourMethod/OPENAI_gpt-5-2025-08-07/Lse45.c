#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>

#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif

#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif

#ifndef O_DIRECTORY
#define O_DIRECTORY 0
#endif

#define MAX_SIZE (1u << 20)         /* 1 MiB */
#define MAX_COMPONENTS 64
#define MAX_COMPONENT_LEN 255

static int is_safe_component(const char* s) {
    size_t len = strlen(s);
    if (len == 0 || len > MAX_COMPONENT_LEN) return 0;
    if (strcmp(s, ".") == 0 || strcmp(s, "..") == 0) return 0;
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)s[i];
        if (!(isalnum(c) || c == '.' || c == '-' || c == '_' || c == ' ')) {
            return 0;
        }
    }
    return 1;
}

static int split_components(const char* rel, char comps[][MAX_COMPONENT_LEN + 1], size_t* count) {
    *count = 0;
    const char* p = rel;
    size_t rel_len = strlen(rel);
    if (rel_len == 0 || rel_len > 4096) return -1;
    if (rel[0] == '/') return -1;
    size_t i = 0;
    while (i < rel_len) {
        if (*count >= MAX_COMPONENTS) return -1;
        size_t j = 0;
        while (i < rel_len && rel[i] != '/') {
            if (j >= MAX_COMPONENT_LEN) return -1;
            if (rel[i] == '\0') return -1;
            comps[*count][j++] = rel[i++];
        }
        comps[*count][j] = '\0';
        if (!is_safe_component(comps[*count])) return -1;
        (*count)++;
        if (i < rel_len && rel[i] == '/') {
            i++;
            if (i < rel_len && rel[i] == '/') return -1; /* no empty component */
        }
    }
    return 0;
}

/* Open directories component-by-component with O_NOFOLLOW; finally open file */
static int open_file_nofollow_all(int basefd, const char* rel, char* errbuf, size_t errlen) {
    char comps[MAX_COMPONENTS][MAX_COMPONENT_LEN + 1];
    size_t n = 0;
    if (split_components(rel, comps, &n) != 0 || n == 0) {
        snprintf(errbuf, errlen, "Invalid path");
        return -1;
    }

    int dfd = basefd;
    int fd = -1;
    for (size_t i = 0; i + 1 < n; ++i) {
        int nd = openat(dfd, comps[i], O_RDONLY | O_CLOEXEC | O_NOFOLLOW | O_DIRECTORY);
        if (nd < 0) {
            snprintf(errbuf, errlen, "Open directory failed");
            return -1;
        }
        if (dfd != basefd) close(dfd);
        dfd = nd;
    }

    fd = openat(dfd, comps[n - 1], O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (dfd != basefd) close(dfd);
    if (fd < 0) {
        snprintf(errbuf, errlen, "Open file failed");
        return -1;
    }
    return fd;
}

static int read_safe_file(const char* base_dir, const char* requested_path, unsigned char** out_buf, size_t* out_len) {
    if (!base_dir || !requested_path || !out_buf || !out_len) return -1;

    int basefd = open(base_dir, O_RDONLY | O_CLOEXEC | O_NOFOLLOW | O_DIRECTORY);
    if (basefd < 0) {
        return -1;
    }

    char err[64];
    int fd = open_file_nofollow_all(basefd, requested_path, err, sizeof(err));
    close(basefd);
    if (fd < 0) {
        return -1;
    }

    struct stat st;
    if (fstat(fd, &st) != 0) {
        close(fd);
        return -1;
    }
    if (!S_ISREG(st.st_mode)) {
        close(fd);
        return -1;
    }
    if (st.st_size < 0 || (unsigned long long)st.st_size > MAX_SIZE) {
        close(fd);
        return -1;
    }

    size_t to_read = (size_t)st.st_size;
    unsigned char* buf = (unsigned char*)malloc(to_read ? to_read : 1);
    if (!buf) {
        close(fd);
        return -1;
    }

    size_t off = 0;
    while (off < to_read) {
        ssize_t r = read(fd, buf + off, to_read - off);
        if (r < 0) {
            free(buf);
            close(fd);
            return -1;
        }
        if (r == 0) break;
        off += (size_t)r;
    }
    close(fd);
    *out_buf = buf;
    *out_len = off;
    return 0;
}

static int write_file(const char* path, const char* data) {
    /* Create parent directories if needed */
    char tmp[4096];
    strncpy(tmp, path, sizeof(tmp) - 1);
    tmp[sizeof(tmp) - 1] = '\0';
    char* slash = strrchr(tmp, '/');
    if (slash) {
        *slash = '\0';
        /* Recursively create directories */
        char seg[4096];
        size_t len = strlen(tmp);
        size_t j = 0;
        for (size_t i = 0; i < len; ++i) {
            seg[j++] = tmp[i];
            seg[j] = '\0';
            if (tmp[i] == '/') {
                mkdir(seg, 0700);
            }
        }
        mkdir(tmp, 0700);
    }
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0600);
    if (fd < 0) return -1;
    ssize_t w = write(fd, data, (int)strlen(data));
    (void)w;
    fsync(fd);
    close(fd);
    return 0;
}

int main(void) {
    char tmpl[] = "/tmp/safe_dir_XXXXXX";
    char* base = mkdtemp(tmpl);
    if (!base) {
        printf("Initialization failed\n");
        return 0;
    }

    char path1[512];
    char path2[512];
    snprintf(path1, sizeof(path1), "%s/a.txt", base);
    snprintf(path2, sizeof(path2), "%s/sub/x.txt", base);

    mkdir((strcat(strcpy((char[512]){0}, base), "/sub")), 0700); /* create sub dir */
    write_file(path1, "Hello from a.txt");
    write_file(path2, "Nested file content");

    /* Create outside and symlink */
    char outside[512];
    snprintf(outside, sizeof(outside), "%s/../outside.txt", base);
    write_file(outside, "outside");
    char evil_link[512];
    snprintf(evil_link, sizeof(evil_link), "%s/evil_link", base);
    unlink(evil_link);
    symlink(outside, evil_link);

    const char* tests[5] = {
        "a.txt",        /* valid */
        "sub/x.txt",    /* valid nested */
        "../etc/passwd",/* traversal */
        "evil_link",    /* symlink */
        "sub"           /* directory */
    };

    for (int i = 0; i < 5; ++i) {
        unsigned char* data = NULL;
        size_t len = 0;
        if (read_safe_file(base, tests[i], &data, &len) == 0) {
            size_t show = len < 80 ? len : 80;
            fwrite("OK: ", 1, 4, stdout);
            fwrite(tests[i], 1, strlen(tests[i]), stdout);
            fwrite(" => ", 1, 4, stdout);
            fwrite(data, 1, show, stdout);
            fwrite("\n", 1, 1, stdout);
            free(data);
        } else {
            fwrite("FAIL: ", 1, 6, stdout);
            fwrite(tests[i], 1, strlen(tests[i]), stdout);
            fwrite("\n", 1, 1, stdout);
        }
    }

    return 0;
}