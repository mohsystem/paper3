#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

#ifdef __APPLE__
#include <CommonCrypto/CommonCrypto.h>
#endif

#define MAX_SIZE (5u * 1024u * 1024u)

static int is_allowed_ext(const char* ext) {
    const char* allowed[] = {"txt","pdf","png","jpg","jpeg","gif","bin"};
    for (size_t i = 0; i < sizeof(allowed)/sizeof(allowed[0]); ++i) {
        if (strcmp(ext, allowed[i]) == 0) return 1;
    }
    return 0;
}

static void sanitize_filename(const char* in, char* out, size_t outsz) {
    size_t j = 0;
    for (size_t i = 0; in[i] != '\0' && j + 1 < outsz; ++i) {
        char c = in[i];
        if ((c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') ||
            c == '.' || c == '_' || c == '-') {
            out[j++] = c;
        } else {
            out[j++] = '_';
        }
        if (j >= 255) break;
    }
    out[j] = '\0';
    // remove leading dots
    size_t k = 0;
    while (out[k] == '.') k++;
    if (k > 0) memmove(out, out + k, j - k + 1);
    if (out[0] == '\0') {
        snprintf(out, outsz, "file");
    }
}

static void ext_or_default(const char* filename, char* ext_out, size_t outsz, const char* defext) {
    const char* dot = strrchr(filename, '.');
    if (dot && *(dot+1)) {
        // lowercase
        size_t i = 0;
        const char* p = dot + 1;
        while (*p && i + 1 < outsz) {
            char c = *p++;
            if (c >= 'A' && c <= 'Z') c = (char)(c - 'A' + 'a');
            ext_out[i++] = c;
        }
        ext_out[i] = '\0';
        if (is_allowed_ext(ext_out)) return;
    }
    snprintf(ext_out, outsz, "%s", defext);
}

static int ensure_dir(const char* dir) {
    // Try to create; if exists, ok. Not recursive robustly for deeply nested, but works for simple paths.
    if (mkdir(dir, 0700) == 0) return 0;
    if (errno == EEXIST) {
        // Set perms best-effort
        chmod(dir, 0700);
        return 0;
    }
    // Attempt to create recursively
    char tmp[1024];
    snprintf(tmp, sizeof(tmp), "%s", dir);
    size_t len = strlen(tmp);
    if (len == 0) return -1;
    if (tmp[len-1] == '/') tmp[len-1] = '\0';
    for (char* p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            if (mkdir(tmp, 0700) != 0 && errno != EEXIST) return -1;
            chmod(tmp, 0700);
            *p = '/';
        }
    }
    if (mkdir(tmp, 0700) != 0 && errno != EEXIST) return -1;
    chmod(tmp, 0700);
    return 0;
}

static int secure_random_bytes(unsigned char* buf, size_t len) {
#if defined(__APPLE__)
    // arc4random_buf is available
    arc4random_buf(buf, len);
    return 0;
#elif defined(__linux__) && defined(SYS_getrandom)
    // try getrandom
    size_t off = 0;
    while (off < len) {
        ssize_t n = syscall(SYS_getrandom, buf + off, len - off, 0);
        if (n < 0) {
            if (errno == EINTR) continue;
            break;
        }
        off += (size_t)n;
    }
    if (off == len) return 0;
#endif
    // Fallback to /dev/urandom
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return -1;
    size_t off = 0;
    while (off < len) {
        ssize_t n = read(fd, buf + off, len - off);
        if (n < 0) {
            if (errno == EINTR) continue;
            close(fd);
            return -1;
        }
        if (n == 0) {
            close(fd);
            return -1;
        }
        off += (size_t)n;
    }
    close(fd);
    return 0;
}

int upload_file(const char* storage_dir, const char* original_filename, const unsigned char* content, size_t len, char* out_msg, size_t out_msg_sz) {
    if (!storage_dir || !original_filename || !content || !out_msg) return -1;
    if (len == 0) {
        snprintf(out_msg, out_msg_sz, "File content is empty.");
        return -2;
    }
    if (len > MAX_SIZE) {
        snprintf(out_msg, out_msg_sz, "File too large. Limit is %u bytes.", (unsigned)MAX_SIZE);
        return -3;
    }

    if (ensure_dir(storage_dir) != 0) {
        snprintf(out_msg, out_msg_sz, "Failed to create storage directory.");
        return -4;
    }

    char safe[256];
    sanitize_filename(original_filename, safe, sizeof(safe));
    char ext[16];
    ext_or_default(safe, ext, sizeof(ext), "bin");

    unsigned char rnd[16];
    if (secure_random_bytes(rnd, sizeof(rnd)) != 0) {
        snprintf(out_msg, out_msg_sz, "Random generation failed.");
        return -5;
    }
    char hex[33];
    for (int i = 0; i < 16; ++i) {
        sprintf(hex + (i*2), "%02x", rnd[i]);
    }
    hex[32] = '\0';

    char unique_name[512];
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    snprintf(unique_name, sizeof(unique_name), "f-%lld-%s.%s", (long long)(ts.tv_sec * 1000LL + ts.tv_nsec / 1000000LL), hex, ext);

    char final_path[1024];
    char tmp_path[1024];
    snprintf(final_path, sizeof(final_path), "%s/%s", storage_dir, unique_name);
    snprintf(tmp_path, sizeof(tmp_path), "%s/upload-%08x.tmp", storage_dir, (unsigned)rnd[0] << 24 | (unsigned)rnd[1] << 16 | (unsigned)rnd[2] << 8 | (unsigned)rnd[3]);

    int fd = open(tmp_path, O_CREAT | O_EXCL | O_WRONLY, 0600);
    if (fd < 0) {
        snprintf(out_msg, out_msg_sz, "Failed to open temp file.");
        return -6;
    }

    size_t off = 0;
    while (off < len) {
        ssize_t n = write(fd, content + off, len - off);
        if (n < 0) {
            if (errno == EINTR) continue;
            close(fd);
            unlink(tmp_path);
            snprintf(out_msg, out_msg_sz, "Failed to write file.");
            return -7;
        }
        off += (size_t)n;
    }
    if (fsync(fd) != 0) {
        // best-effort
    }
    close(fd);

    if (rename(tmp_path, final_path) != 0) {
        // try to remove and retry once
        unlink(final_path);
        if (rename(tmp_path, final_path) != 0) {
            unlink(tmp_path);
            snprintf(out_msg, out_msg_sz, "Failed to move temp file to final destination.");
            return -8;
        }
    }

    chmod(final_path, 0600);

    snprintf(out_msg, out_msg_sz, "Upload successful. Stored as %s (%zu bytes).", unique_name, len);
    return 0;
}

int main(void) {
    const char* storage = "uploads_c";
    unsigned char data1[] = "Hello World";
    unsigned char data2[] = "Not a passwd";
    unsigned char data3[] = {1,2,3,4,5};
    unsigned char data4[] = "binary";
    size_t big_len = MAX_SIZE + 1024;
    unsigned char* big = (unsigned char*)malloc(big_len);
    if (big) {
        for (size_t i = 0; i < big_len; ++i) big[i] = (unsigned char)(i & 0xFF);
    }

    struct {
        const char* name;
        unsigned char* content;
        size_t len;
    } tests[5] = {
        {"hello.txt", data1, sizeof(data1)-1},
        {"../../etc/passwd", data2, sizeof(data2)-1},
        {"image.PNG", data3, sizeof(data3)},
        {"script.exe", data4, sizeof(data4)-1},
        {"..hidden..file..name..pdf", big, big ? big_len : 0}
    };

    for (int i = 0; i < 5; ++i) {
        char msg[512];
        int rc = upload_file(storage, tests[i].name, tests[i].content, tests[i].len, msg, sizeof(msg));
        if (rc == 0) {
            printf("%d: %s\n", i+1, msg);
        } else {
            printf("%d: Upload failed: %s\n", i+1, msg);
        }
    }

    if (big) free(big);
    return 0;
}