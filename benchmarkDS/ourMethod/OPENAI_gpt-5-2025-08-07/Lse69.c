#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <dirent.h>
#include <time.h>

#define MAX_SIZE (8 * 1024 * 1024) /* 8 MiB */

static bool is_safe_filename(const char* name) {
    if (name == NULL) return false;
    size_t len = strlen(name);
    if (len == 0 || len > 255) return false;
    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) return false;
    for (size_t i = 0; i < len; ++i) {
        char c = name[i];
        if (c == '/' || c == '\\') return false;
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9') || c == '.' || c == '_' || c == '-')) {
            return false;
        }
    }
    return true;
}

static char* base64_encode(const unsigned char* data, size_t in_len, size_t* out_len) {
    static const char tbl[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t olen = ((in_len + 2) / 3) * 4;
    char* out = (char*)malloc(olen + 1);
    if (!out) return NULL;
    size_t i = 0, j = 0;
    while (i + 2 < in_len) {
        unsigned int n = (data[i] << 16) | (data[i+1] << 8) | data[i+2];
        out[j++] = tbl[(n >> 18) & 63];
        out[j++] = tbl[(n >> 12) & 63];
        out[j++] = tbl[(n >> 6) & 63];
        out[j++] = tbl[n & 63];
        i += 3;
    }
    if (i < in_len) {
        unsigned int n = data[i] << 16;
        if (i + 1 < in_len) n |= (data[i+1] << 8);
        out[j++] = tbl[(n >> 18) & 63];
        out[j++] = tbl[(n >> 12) & 63];
        if (i + 1 < in_len) {
            out[j++] = tbl[(n >> 6) & 63];
            out[j++] = '=';
        } else {
            out[j++] = '=';
            out[j++] = '=';
        }
    }
    out[j] = '\0';
    if (out_len) *out_len = j;
    return out;
}

static bool ensure_dir(const char* dir) {
    struct stat st;
    if (stat(dir, &st) == 0) {
        return S_ISDIR(st.st_mode);
    }
    if (mkdir(dir, 0700) == 0) return true;
    return false;
}

static char* concat_path(const char* dir, const char* name) {
    size_t ld = strlen(dir), ln = strlen(name);
    size_t total = ld + 1 + ln + 1;
    char* out = (char*)malloc(total);
    if (!out) return NULL;
    snprintf(out, total, "%s/%s", dir, name);
    return out;
}

static char* upload_image(const char* base_dir, const char* input_name, const char* output_name) {
    if (!base_dir || !is_safe_filename(input_name) || !is_safe_filename(output_name)) {
        return NULL;
    }
    if (!ensure_dir(base_dir)) {
        return NULL;
    }

    int dfd = open(base_dir, O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
    if (dfd < 0) {
        return NULL;
    }

    int ifd = openat(dfd, input_name, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (ifd < 0) {
        close(dfd);
        return NULL;
    }

    struct stat st;
    if (fstat(ifd, &st) != 0) {
        close(ifd);
        close(dfd);
        return NULL;
    }
    if (!S_ISREG(st.st_mode)) {
        close(ifd);
        close(dfd);
        return NULL;
    }
    if (st.st_size < 0 || st.st_size > MAX_SIZE) {
        close(ifd);
        close(dfd);
        return NULL;
    }

    size_t fsize = (size_t)st.st_size;
    unsigned char* buf = NULL;
    buf = (unsigned char*)malloc(fsize ? fsize : 1);
    if (!buf) {
        close(ifd);
        close(dfd);
        return NULL;
    }

    size_t off = 0;
    while (off < fsize) {
        ssize_t r = read(ifd, buf + off, fsize - off);
        if (r < 0) {
            if (errno == EINTR) continue;
            free(buf);
            close(ifd);
            close(dfd);
            return NULL;
        }
        if (r == 0) break;
        off += (size_t)r;
    }
    if (off != fsize) {
        free(buf);
        close(ifd);
        close(dfd);
        return NULL;
    }
    close(ifd);

    size_t b64_len = 0;
    char* b64 = base64_encode(buf, fsize, &b64_len);
    free(buf);
    if (!b64) {
        close(dfd);
        return NULL;
    }

    // Create unique temp file
    char tmpname[64];
    int tfd = -1;
    for (int i = 0; i < 10; ++i) {
        unsigned long r = (unsigned long)random();
        snprintf(tmpname, sizeof(tmpname), "enc-%08lx.tmp", r);
        tfd = openat(dfd, tmpname, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW, 0600);
        if (tfd >= 0) break;
        if (errno != EEXIST) {
            free(b64);
            close(dfd);
            return NULL;
        }
    }
    if (tfd < 0) {
        free(b64);
        close(dfd);
        return NULL;
    }

    size_t left = b64_len;
    const char* p = b64;
    while (left > 0) {
        ssize_t w = write(tfd, p, left);
        if (w < 0) {
            if (errno == EINTR) continue;
            close(tfd);
            unlinkat(dfd, tmpname, 0);
            free(b64);
            close(dfd);
            return NULL;
        }
        p += w;
        left -= (size_t)w;
    }
    if (fsync(tfd) != 0) {
        close(tfd);
        unlinkat(dfd, tmpname, 0);
        free(b64);
        close(dfd);
        return NULL;
    }
    if (close(tfd) != 0) {
        unlinkat(dfd, tmpname, 0);
        free(b64);
        close(dfd);
        return NULL;
    }
    free(b64);

    // Publish atomically without overwrite: link + unlink
    if (linkat(dfd, tmpname, dfd, output_name, 0) != 0) {
        unlinkat(dfd, tmpname, 0);
        close(dfd);
        return NULL;
    }
    unlinkat(dfd, tmpname, 0);

    // fsync directory best-effort
    fsync(dfd);
    close(dfd);

    char* outpath = concat_path(base_dir, output_name);
    return outpath; /* caller must free */
}

static bool write_sample_file(const char* base_dir, const char* name, const unsigned char* content, size_t len) {
    if (!is_safe_filename(name)) return false;
    int dfd = open(base_dir, O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
    if (dfd < 0) return false;
    char tmpname[64];
    snprintf(tmpname, sizeof(tmpname), "in-%ld.tmp", (long)getpid());
    int tfd = openat(dfd, tmpname, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC | O_NOFOLLOW, 0600);
    if (tfd < 0) { close(dfd); return false; }
    size_t left = len;
    const unsigned char* p = content;
    while (left > 0) {
        ssize_t w = write(tfd, p, left);
        if (w < 0) {
            if (errno == EINTR) continue;
            close(tfd); unlinkat(dfd, tmpname, 0); close(dfd); return false;
        }
        p += w; left -= (size_t)w;
    }
    fsync(tfd);
    close(tfd);
    if (linkat(dfd, tmpname, dfd, name, 0) != 0) {
        unlinkat(dfd, tmpname, 0); close(dfd); return false;
    }
    unlinkat(dfd, tmpname, 0);
    fsync(dfd);
    close(dfd);
    return true;
}

int main(void) {
    const char* base = "base_c";
    if (!ensure_dir(base)) {
        printf("Setup failed\n");
        return 0;
    }
    srandom((unsigned)time(NULL));
    unsigned char c1[32], c2[64], c3[128], c4[16], c5[256];
    for (size_t i = 0; i < sizeof(c1); ++i) c1[i] = (unsigned char)(random() & 0xFF);
    for (size_t i = 0; i < sizeof(c2); ++i) c2[i] = (unsigned char)(random() & 0xFF);
    for (size_t i = 0; i < sizeof(c3); ++i) c3[i] = (unsigned char)(random() & 0xFF);
    for (size_t i = 0; i < sizeof(c4); ++i) c4[i] = (unsigned char)(random() & 0xFF);
    for (size_t i = 0; i < sizeof(c5); ++i) c5[i] = (unsigned char)(random() & 0xFF);

    write_sample_file(base, "img1.bin", c1, sizeof(c1));
    write_sample_file(base, "img2.jpg", c2, sizeof(c2));
    write_sample_file(base, "img3.png", c3, sizeof(c3));
    write_sample_file(base, "icon4.gif", c4, sizeof(c4));
    write_sample_file(base, "photo5.bmp", c5, sizeof(c5));

    const char* inputs[5] = {"img1.bin", "img2.jpg", "img3.png", "icon4.gif", "photo5.bmp"};
    const char* outputs[5] = {"img1.b64", "img2.b64", "img3.b64", "icon4.b64", "photo5.b64"};
    for (int i = 0; i < 5; ++i) {
        char* res = upload_image(base, inputs[i], outputs[i]);
        if (res) {
            printf("OK: %s\n", res);
            free(res);
        } else {
            printf("FAIL\n");
        }
    }
    return 0;
}