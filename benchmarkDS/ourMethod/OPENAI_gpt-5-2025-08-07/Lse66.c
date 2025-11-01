#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_PDF_SIZE (10 * 1024 * 1024) /* 10 MiB */

static int is_dir_mode(mode_t m) { return S_ISDIR(m); }
static int is_reg_mode(mode_t m) { return S_ISREG(m); }

static void hex_random(char* out, size_t outlen) {
    /* Not cryptographically strong, but sufficient for temp name randomness.
       Use urandom if available. */
    int fd = open("/dev/urandom", O_RDONLY);
    unsigned char buf[16];
    if (fd >= 0) {
        ssize_t r = read(fd, buf, sizeof(buf));
        (void)r;
        close(fd);
    } else {
        for (size_t i = 0; i < sizeof(buf); ++i) buf[i] = (unsigned char)(rand() & 0xFF);
    }
    static const char* hex = "0123456789abcdef";
    size_t j = 0;
    for (size_t i = 0; i < sizeof(buf) && j + 1 < outlen; ++i) {
        if (j + 2 >= outlen) break;
        out[j++] = hex[(buf[i] >> 4) & 0xF];
        out[j++] = hex[buf[i] & 0xF];
    }
    if (j < outlen) out[j] = '\0';
}

static char* sanitize_filename(const char* name) {
    size_t n = strlen(name);
    const char* base = name;
    for (size_t i = 0; i < n; ++i) {
        if (name[i] == '/' || name[i] == '\\') base = name + i + 1;
    }
    char* out = (char*)malloc(256);
    if (!out) return NULL;
    size_t j = 0;
    for (size_t i = 0; base[i] && j < 128; ++i) {
        unsigned char ch = (unsigned char)base[i];
        char l = (char)tolower(ch);
        if ((l >= 'a' && l <= 'z') || (l >= '0' && l <= '9') || l == '.' || l == '_' || l == '-') {
            out[j++] = l;
        } else {
            out[j++] = '_';
        }
    }
    if (j == 0) {
        strcpy(out, "upload.pdf");
        return out;
    }
    out[j] = '\0';
    /* Collapse .. */
    while (strstr(out, "..") != NULL) {
        char* p = strstr(out, "..");
        p[0] = '.';
        memmove(p + 1, p + 2, strlen(p + 2) + 1);
    }
    /* Ensure .pdf */
    size_t len = strlen(out);
    int has_pdf = 0;
    if (len >= 4 && strcmp(out + len - 4, ".pdf") == 0) has_pdf = 1;
    if (!has_pdf) {
        while (len > 0 && out[len - 1] == '.') {
            out[--len] = '\0';
        }
        if (len + 4 + 1 >= 256) len = 256 - 5;
        strcpy(out + len, ".pdf");
    }
    if (out[0] == '.') {
        /* prepend 'u' */
        if (strlen(out) + 1 < 256) {
            memmove(out + 1, out, strlen(out) + 1);
            out[0] = 'u';
        } else {
            out[0] = 'u';
        }
    }
    return out;
}

static char* save_pdf(const unsigned char* data, size_t data_len, const char* original_filename, const char* base_dir) {
    if (!data || !original_filename || !base_dir) return NULL;
    if (data_len < 5 || data_len > MAX_PDF_SIZE) return NULL;
    if (memcmp(data, "%PDF-", 5) != 0) return NULL;

    int dflags = O_RDONLY;
#ifdef O_DIRECTORY
    dflags |= O_DIRECTORY;
#endif
#ifdef O_CLOEXEC
    dflags |= O_CLOEXEC;
#endif
#ifdef O_NOFOLLOW
    dflags |= O_NOFOLLOW;
#endif

    int dfd = open(base_dir, dflags);
    if (dfd < 0) return NULL;

    struct stat dst;
    if (fstat(dfd, &dst) != 0 || !is_dir_mode(dst.st_mode)) {
        close(dfd);
        return NULL;
    }

    char* dest = sanitize_filename(original_filename);
    if (!dest) {
        close(dfd);
        return NULL;
    }
    if (strchr(dest, '/') != NULL || strchr(dest, '\\') != NULL) {
        free(dest);
        close(dfd);
        return NULL;
    }

    char tmpname[128];
    strcpy(tmpname, "upload_");
    hex_random(tmpname + strlen(tmpname), sizeof(tmpname) - strlen(tmpname));
    strncat(tmpname, ".tmp", sizeof(tmpname) - strlen(tmpname) - 1);

    int fflags = O_WRONLY | O_CREAT | O_EXCL;
#ifdef O_CLOEXEC
    fflags |= O_CLOEXEC;
#endif
#ifdef O_NOFOLLOW
    fflags |= O_NOFOLLOW;
#endif

    int fd = openat(dfd, tmpname, fflags, 0600);
    if (fd < 0) {
        free(dest);
        close(dfd);
        return NULL;
    }

    struct stat fst;
    if (fstat(fd, &fst) != 0 || !is_reg_mode(fst.st_mode)) {
        close(fd);
        unlinkat(dfd, tmpname, 0);
        free(dest);
        close(dfd);
        return NULL;
    }

    size_t total = 0;
    while (total < data_len) {
        ssize_t w = write(fd, data + total, data_len - total);
        if (w <= 0) {
            close(fd);
            unlinkat(dfd, tmpname, 0);
            free(dest);
            close(dfd);
            return NULL;
        }
        total += (size_t)w;
        if (total > MAX_PDF_SIZE) {
            close(fd);
            unlinkat(dfd, tmpname, 0);
            free(dest);
            close(dfd);
            return NULL;
        }
    }

    if (fsync(fd) != 0) {
        close(fd);
        unlinkat(dfd, tmpname, 0);
        free(dest);
        close(dfd);
        return NULL;
    }
    close(fd);

    if (linkat(dfd, tmpname, dfd, dest, 0) != 0) {
        unlinkat(dfd, tmpname, 0);
        free(dest);
        close(dfd);
        return NULL;
    }
    unlinkat(dfd, tmpname, 0);

    /* Fsync directory best-effort */
    fsync(dfd);
    close(dfd);

    size_t outlen = strlen(base_dir) + 1 + strlen(dest) + 1;
    char* out = (char*)malloc(outlen);
    if (!out) {
        free(dest);
        return NULL;
    }
    snprintf(out, outlen, "%s/%s", base_dir, dest);
    free(dest);
    return out;
}

static void print_result(const char* label, char* path) {
    if (path) {
        printf("%s: %s\n", label, path);
        free(path);
    } else {
        printf("%s: FAIL\n", label);
    }
}

int main(void) {
    /* Ensure uploads directory exists */
    mkdir("uploads", 0700);

    /* Test 1: Valid PDF */
    {
        const char* s = "%PDF-1.7\n...";
        char* p = save_pdf((const unsigned char*)s, strlen(s), "report.pdf", "uploads");
        print_result("OK1", p);
    }

    /* Test 2: Path traversal */
    {
        const char* s = "%PDF-1.7\n...";
        char* p = save_pdf((const unsigned char*)s, strlen(s), "../../evil.pdf", "uploads");
        print_result("OK2", p);
    }

    /* Test 3: Wrong extension */
    {
        const char* s = "%PDF-1.7\n...";
        char* p = save_pdf((const unsigned char*)s, strlen(s), "image.png", "uploads");
        print_result("OK3", p);
    }

    /* Test 4: Too big */
    {
        size_t n = MAX_PDF_SIZE + 1;
        unsigned char* buf = (unsigned char*)malloc(n);
        if (!buf) {
            printf("FAIL4: alloc\n");
        } else {
            memset(buf, 'A', n);
            memcpy(buf, "%PDF-", 5);
            char* p = save_pdf(buf, n, "big.pdf", "uploads");
            print_result("OK4", p);
            free(buf);
        }
    }

    /* Test 5: Odd characters filename */
    {
        const char* s = "%PDF-1.4\n...";
        char* p = save_pdf((const unsigned char*)s, strlen(s), "a..//b??.pdf", "uploads");
        print_result("OK5", p);
    }

    return 0;
}