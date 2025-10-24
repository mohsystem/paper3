#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#if defined(_WIN32)
  #define NOMINMAX
  #include <windows.h>
  #include <bcrypt.h>
  #pragma comment(lib, "bcrypt.lib")
  #include <io.h>
#else
  #include <unistd.h>
  #include <fcntl.h>
  #include <errno.h>
  #include <sys/stat.h>
  #if defined(__linux__)
    #include <sys/random.h>
  #endif
#endif

static const char* ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-";

static int secure_random_bytes(uint8_t* buf, size_t len) {
#if defined(_WIN32)
    NTSTATUS status = BCryptGenRandom(NULL, buf, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    return status == 0 ? 1 : 0;
#else
  #if defined(__linux__)
    size_t total = 0;
    while (total < len) {
        ssize_t n = getrandom(buf + total, len - total, 0);
        if (n < 0) {
            if (errno == EINTR) continue;
            return 0;
        }
        total += (size_t)n;
    }
    return 1;
  #else
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return 0;
    size_t total = 0;
    while (total < len) {
        ssize_t n = read(fd, buf + total, len - total);
        if (n < 0) {
            if (errno == EINTR) continue;
            close(fd);
            return 0;
        }
        total += (size_t)n;
    }
    close(fd);
    return 1;
  #endif
#endif
}

static size_t random_index(size_t bound) {
    if (bound == 0) return 0;
    uint64_t x;
    const uint64_t limit = (UINT64_MAX / bound) * bound;
    for (;;) {
        if (!secure_random_bytes((uint8_t*)&x, sizeof(x))) {
            // Fallback abort on failure for security
            fprintf(stderr, "secure_random_bytes failed\n");
            exit(1);
        }
        if (x < limit) {
            return (size_t)(x % bound);
        }
    }
}

static char* generate_random_string(size_t length) {
    char* s = (char*)malloc(length + 1);
    if (!s) return NULL;
    size_t alpha_len = strlen(ALPHABET);
    for (size_t i = 0; i < length; ++i) {
        size_t idx = random_index(alpha_len);
        s[i] = ALPHABET[idx];
    }
    s[length] = '\0';
    return s;
}

static char* to_unicode_escapes(const char* s) {
    size_t n = strlen(s);
    size_t out_len = n * 6;
    char* out = (char*)malloc(out_len + 1);
    if (!out) return NULL;
    char* p = out;
    for (size_t i = 0; i < n; ++i) {
        unsigned char c = (unsigned char)s[i];
        // Write as \u00XX
        // Ensure uppercase hex
        static const char HEX[] = "0123456789ABCDEF";
        *p++ = '\\';
        *p++ = 'u';
        *p++ = '0';
        *p++ = '0';
        *p++ = HEX[(c >> 4) & 0xF];
        *p++ = HEX[c & 0xF];
    }
    *p = '\0';
    return out;
}

static char* create_temp_unicode_file(size_t length) {
    if (length == 0) return NULL;

    char* rnd = generate_random_string(length);
    if (!rnd) return NULL;

    char* escaped = to_unicode_escapes(rnd);
    free(rnd);
    if (!escaped) return NULL;

    char* path = NULL;

#if defined(_WIN32)
    char tempPath[MAX_PATH];
    DWORD tplen = GetTempPathA(MAX_PATH, tempPath);
    if (tplen == 0 || tplen > MAX_PATH) {
        free(escaped);
        return NULL;
    }
    char tempFile[MAX_PATH];
    if (GetTempFileNameA(tempPath, "t88", 0, tempFile) == 0) {
        free(escaped);
        return NULL;
    }

    FILE* f = fopen(tempFile, "wb");
    if (!f) {
        free(escaped);
        return NULL;
    }
    size_t len = strlen(escaped);
    size_t written = fwrite(escaped, 1, len, f);
    fflush(f);
    free(escaped);
    if (written != len) {
        fclose(f);
        return NULL;
    }
    fclose(f);

    path = _strdup(tempFile);
    if (!path) return NULL;

#else
    const char* dir = getenv("TMPDIR");
    if (!dir || *dir == '\0') dir = "/tmp";
    size_t dirlen = strlen(dir);
    const char* prefix = "/task88_XXXXXX";
    size_t prelen = strlen(prefix);

    char* tmpl = (char*)malloc(dirlen + prelen + 1);
    if (!tmpl) {
        free(escaped);
        return NULL;
    }
    memcpy(tmpl, dir, dirlen);
    memcpy(tmpl + dirlen, prefix, prelen + 1);

    int fd = mkstemp(tmpl);
    if (fd == -1) {
        free(escaped);
        free(tmpl);
        return NULL;
    }

    // Restrict permissions to 0600
    fchmod(fd, S_IRUSR | S_IWUSR);

    size_t total = 0;
    size_t len = strlen(escaped);
    while (total < len) {
        ssize_t w = write(fd, escaped + total, len - total);
        if (w < 0) {
            if (errno == EINTR) continue;
            close(fd);
            free(escaped);
            free(tmpl);
            return NULL;
        }
        total += (size_t)w;
    }
    fsync(fd);
    close(fd);
    free(escaped);

    path = strdup(tmpl);
    free(tmpl);
    if (!path) return NULL;
#endif

    return path;
}

int main(void) {
    // 5 test cases
    char* p1 = create_temp_unicode_file(8);
    char* p2 = create_temp_unicode_file(12);
    char* p3 = create_temp_unicode_file(1);
    char* p4 = create_temp_unicode_file(32);
    char* p5 = create_temp_unicode_file(64);

    if (p1) { printf("%s\n", p1); free(p1); }
    if (p2) { printf("%s\n", p2); free(p2); }
    if (p3) { printf("%s\n", p3); free(p3); }
    if (p4) { printf("%s\n", p4); free(p4); }
    if (p5) { printf("%s\n", p5); free(p5); }

    return 0;
}