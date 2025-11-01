#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef _WIN32
#include <direct.h>
#define mkdir_p(path, mode) _mkdir(path)
#else
#define mkdir_p(path, mode) mkdir(path, mode)
#endif

#define MAX_SIZE (1024 * 1024) // 1 MB

static int is_pdf_header(const unsigned char* data, size_t len) {
    if (len < 5) return 0;
    return data[0]=='%' && data[1]=='P' && data[2]=='D' && data[3]=='F' && data[4]=='-';
}

static void sanitize_filename(const char* in, char* out, size_t out_len) {
    if (!in || !*in) {
        snprintf(out, out_len, "file");
        return;
    }
    // keep base name only
    const char* base = in;
    const char* s = in;
    while (*s) {
        if (*s == '/' || *s == '\\') base = s + 1;
        s++;
    }
    // copy and sanitize
    size_t j = 0;
    size_t n = strlen(base);
    while (n > 0 && base[n-1] == '.') n--; // trim trailing dots
    for (size_t i = 0; i < n && j + 1 < out_len; i++) {
        unsigned char c = (unsigned char)base[i];
        if ((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') ||
            c == '-' || c == '_' || c == '.') {
            out[j++] = (char)c;
        } else {
            out[j++] = '_';
        }
        if (j >= 100) break; // limit base length
    }
    out[j] = '\0';
    // strip .pdf if present at end (case-insensitive)
    size_t L = strlen(out);
    if (L >= 4) {
        char *p = out + (L - 4);
        if ((p[0]=='.' || p[0]=='.') &&
            (p[1]=='p' || p[1]=='P') &&
            (p[2]=='d' || p[2]=='D') &&
            (p[3]=='f' || p[3]=='F')) {
            p[0] = '\0';
        }
    }
    if (out[0] == '\0') {
        snprintf(out, out_len, "file");
    }
}

static int get_random_bytes(unsigned char* buf, size_t len) {
#ifdef _WIN32
    // Fallback on Windows: use rand (not cryptographically secure)
    srand((unsigned)time(NULL) ^ (unsigned)getpid());
    for (size_t i = 0; i < len; i++) buf[i] = (unsigned char)(rand() & 0xFF);
    return 0;
#else
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return -1;
    size_t off = 0;
    while (off < len) {
        ssize_t r = read(fd, buf + off, len - off);
        if (r <= 0) { close(fd); return -1; }
        off += (size_t)r;
    }
    close(fd);
    return 0;
#endif
}

static void hex_encode(const unsigned char* in, size_t len, char* out, size_t out_len) {
    static const char* hex = "0123456789abcdef";
    size_t j = 0;
    for (size_t i = 0; i < len && j + 2 < out_len; i++) {
        unsigned v = in[i];
        out[j++] = hex[(v >> 4) & 0xF];
        out[j++] = hex[v & 0xF];
    }
    out[j] = '\0';
}

int save_pdf(const unsigned char* data, size_t len, const char* original_filename, char* out_path, size_t out_path_len) {
    if (!data || len == 0) return -1;
    if (len > MAX_SIZE) return -2;
    if (!is_pdf_header(data, len)) return -3;

    // Create uploads directory
    if (mkdir_p("uploads", 0700) != 0) {
        if (errno != EEXIST) {
            // proceed if already exists
        }
    }
#ifndef _WIN32
    chmod("uploads", 0700);
#endif

    char base[128];
    sanitize_filename(original_filename ? original_filename : "file.pdf", base, sizeof(base));

    unsigned char rnd[8];
    if (get_random_bytes(rnd, sizeof(rnd)) != 0) {
        // fallback to time-based randomness
        srand((unsigned)time(NULL) ^ (unsigned)getpid());
        for (size_t i = 0; i < sizeof(rnd); i++) rnd[i] = (unsigned char)(rand() & 0xFF);
    }
    char rnd_hex[32];
    hex_encode(rnd, sizeof(rnd), rnd_hex, sizeof(rnd_hex));

    char final_name[512];
    snprintf(final_name, sizeof(final_name), "%s_%ld_%s.pdf", base, (long)time(NULL), rnd_hex);

    char temp_name[512];
    snprintf(temp_name, sizeof(temp_name), "uploads/upload_%s.tmp", rnd_hex);

    // Write to temp file
#ifdef _WIN32
    int fd = _open(temp_name, _O_CREAT | _O_EXCL | _O_WRONLY | _O_BINARY, _S_IREAD | _S_IWRITE);
#else
    int fd = open(temp_name, O_CREAT | O_EXCL | O_WRONLY, 0600);
#endif
    if (fd < 0) return -4;

    size_t off = 0;
    while (off < len) {
        ssize_t w = write(fd, data + off, len - off);
        if (w <= 0) {
            close(fd);
            unlink(temp_name);
            return -5;
        }
        off += (size_t)w;
    }
    fsync(fd);
    close(fd);
#ifndef _WIN32
    chmod(temp_name, 0600);
#endif

    char final_path[1024];
    snprintf(final_path, sizeof(final_path), "uploads/%s", final_name);

    if (rename(temp_name, final_path) != 0) {
        unlink(temp_name);
        return -6;
    }
#ifndef _WIN32
    chmod(final_path, 0600);
#endif

    if (out_path && out_path_len > 0) {
        realpath(final_path, out_path);
    }
    return 0;
}

int main(void) {
    // Test 1: Valid PDF
    const char* pdf1 = "%PDF-1.7\n%Test\n";
    char out[1024] = {0};
    printf("Test1: %d %s\n", save_pdf((const unsigned char*)pdf1, strlen(pdf1), "test.pdf", out, sizeof(out)), out);

    // Test 2: Empty
    memset(out, 0, sizeof(out));
    printf("Test2: %d\n", save_pdf((const unsigned char*)"", 0, "test.pdf", out, sizeof(out)));

    // Test 3: Invalid header
    const char* bad = "HELLO";
    memset(out, 0, sizeof(out));
    printf("Test3: %d\n", save_pdf((const unsigned char*)bad, strlen(bad), "doc.pdf", out, sizeof(out)));

    // Test 4: Path traversal filename
    memset(out, 0, sizeof(out));
    printf("Test4: %d %s\n", save_pdf((const unsigned char*)pdf1, strlen(pdf1), "../evil.pdf", out, sizeof(out)), out);

    // Test 5: Too large
    size_t big_len = MAX_SIZE + 10;
    unsigned char* big = (unsigned char*)malloc(big_len);
    if (big) {
        memset(big, 'A', big_len);
        big[0] = '%'; big[1] = 'P'; big[2] = 'D'; big[3] = 'F'; big[4] = '-';
        memset(out, 0, sizeof(out));
        printf("Test5: %d\n", save_pdf(big, big_len, "big.pdf", out, sizeof(out)));
        free(big);
    } else {
        printf("Test5: memory alloc failed\n");
    }

    return 0;
}