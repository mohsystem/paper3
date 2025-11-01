#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define MAX_SIZE (1024 * 1024) /* 1 MiB */

static int validate_rel_path(const char* p) {
    if (p == NULL) return 0;
    size_t n = strlen(p);
    if (n == 0 || n > 4096) return 0;
    if (p[0] == '/') return 0;
    if (strchr(p, '\0') == NULL) return 0;
    for (size_t i = 0; i < n; i++) {
        if (p[i] == '\\') return 0;
    }
    // Split and check segments
    char* tmp = (char*)malloc(n + 1);
    if (!tmp) return 0;
    memcpy(tmp, p, n + 1);
    char* saveptr = NULL;
    char* seg = strtok_r(tmp, "/", &saveptr);
    int seen = 0;
    while (seg) {
        if (seg[0] == '\0' || strcmp(seg, ".") == 0 || strcmp(seg, "..") == 0) { free(tmp); return 0; }
        for (char* c = seg; *c; ++c) {
            if (!( (*c >= 'a' && *c <= 'z') || (*c >= 'A' && *c <= 'Z') || (*c >= '0' && *c <= '9') || *c == '.' || *c == '_' || *c == '-' )) {
                free(tmp);
                return 0;
            }
        }
        seen = 1;
        seg = strtok_r(NULL, "/", &saveptr);
    }
    free(tmp);
    return seen;
}

static const char* guess_content_type(const char* name) {
    const char* dot = strrchr(name, '.');
    const char* ext = dot ? dot + 1 : "";
    if (strcasecmp(ext, "png") == 0) return "image/png";
    if (strcasecmp(ext, "jpg") == 0 || strcasecmp(ext, "jpeg") == 0) return "image/jpeg";
    if (strcasecmp(ext, "gif") == 0) return "image/gif";
    if (strcasecmp(ext, "svg") == 0) return "image/svg+xml";
    if (strcasecmp(ext, "txt") == 0) return "text/plain; charset=utf-8";
    return "application/octet-stream";
}

/*
 Returns HTTP-like status code.
 On success (*out_buf, *out_len, out_type) are set. Caller must free(*out_buf).
*/
int fetch_image_file(const char* base_dir, const char* relpath, unsigned char** out_buf, size_t* out_len, char* out_type, size_t out_type_len) {
    if (!validate_rel_path(relpath)) {
        return 400;
    }
    int dirfd = open(base_dir, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (dirfd < 0) return 500;

    int fd = -1;
    struct stat st;
    memset(&st, 0, sizeof(st));
    int status = 200;

    do {
        int flags = O_RDONLY | O_CLOEXEC;
#ifdef O_NOFOLLOW
        flags |= O_NOFOLLOW;
#endif
        fd = openat(dirfd, relpath, flags);
        if (fd < 0) {
            if (errno == ELOOP || errno == ENOENT) status = 404;
            else status = 404;
            break;
        }
        if (fstat(fd, &st) != 0) { status = 404; break; }
        if (!S_ISREG(st.st_mode)) { status = 404; break; }
        if (st.st_size < 0 || (size_t)st.st_size > MAX_SIZE) { status = 413; break; }

        unsigned char* buf = (unsigned char*)malloc((size_t)st.st_size);
        if (!buf) { status = 500; break; }

        size_t need = (size_t)st.st_size;
        size_t got = 0;
        while (got < need) {
            ssize_t r = read(fd, buf + got, need - got);
            if (r < 0) { free(buf); status = 500; goto done; }
            if (r == 0) break;
            got += (size_t)r;
            if (got > MAX_SIZE) { free(buf); status = 413; goto done; }
        }
        *out_buf = buf;
        *out_len = got;

        const char* ct = guess_content_type(relpath);
        if (out_type && out_type_len > 0) {
            snprintf(out_type, out_type_len, "%s", ct);
        }
        status = 200;
    } while (0);

done:
    if (fd >= 0) close(fd);
    close(dirfd);
    return status;
}

static int write_file(const char* path, const unsigned char* data, size_t len) {
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0600);
    if (fd < 0) return -1;
    size_t off = 0;
    while (off < len) {
        ssize_t w = write(fd, data + off, len - off);
        if (w < 0) { close(fd); return -1; }
        off += (size_t)w;
    }
    close(fd);
    return 0;
}

int main(void) {
    char cwd[4096];
    if (!getcwd(cwd, sizeof(cwd))) return 1;

    char images_dir[8192];
    snprintf(images_dir, sizeof(images_dir), "%s/images", cwd);
    mkdir(images_dir, 0700);

    // hello.txt
    {
        char p[8192]; snprintf(p, sizeof(p), "%s/hello.txt", images_dir);
        const unsigned char msg[] = "Hello World!";
        write_file(p, msg, sizeof(msg)-1);
    }

    // big.bin > 1 MiB
    {
        char p[8192]; snprintf(p, sizeof(p), "%s/big.bin", images_dir);
        unsigned char* big = (unsigned char*)malloc(MAX_SIZE + 1);
        if (!big) return 1;
        for (size_t i = 0; i < MAX_SIZE + 1; i++) big[i] = (unsigned char)(i & 0xFF);
        write_file(p, big, MAX_SIZE + 1);
        free(big);
    }

    // outside.txt and symlink
    {
        char p[8192]; snprintf(p, sizeof(p), "%s/outside.txt", cwd);
        const unsigned char omsg[] = "outside";
        write_file(p, omsg, sizeof(omsg)-1);

        char linkp[8192]; snprintf(linkp, sizeof(linkp), "%s/link_to_outside.txt", images_dir);
        unlink(linkp);
#ifdef __unix__
        symlink("../outside.txt", linkp);
#endif
    }

    const char* tests[] = {
        "hello.txt",
        "missing.txt",
        "../outside.txt",
        "link_to_outside.txt",
        "big.bin"
    };

    for (size_t i = 0; i < 5; i++) {
        unsigned char* buf = NULL;
        size_t len = 0;
        char ctype[128];
        int code = fetch_image_file(images_dir, tests[i], &buf, &len, ctype, sizeof(ctype));
        printf("%s -> %d (len=%zu, type=%s)\n", tests[i], code, len, (code == 200 ? ctype : "-"));
        free(buf);
    }

    return 0;
}