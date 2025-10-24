#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>
#include <time.h>

// Ranges: digits, uppercase, lowercase, Latin-1 supplement, Cyrillic, Emoji
typedef struct { uint32_t lo; uint32_t hi; } Range;
static const Range RANGES[] = {
    {0x0030, 0x0039}, {0x0041, 0x005A}, {0x0061, 0x007A},
    {0x00C0, 0x00FF}, {0x0400, 0x04FF}, {0x1F600, 0x1F64F}
};
static const size_t RANGES_COUNT = sizeof(RANGES)/sizeof(RANGES[0]);

static bool is_surrogate(uint32_t cp) {
    return cp >= 0xD800 && cp <= 0xDFFF;
}

static int utf8_encode_cp(uint32_t cp, char out[4]) {
    if (cp <= 0x7F) {
        out[0] = (char)cp;
        return 1;
    } else if (cp <= 0x7FF) {
        out[0] = (char)(0xC0 | ((cp >> 6) & 0x1F));
        out[1] = (char)(0x80 | (cp & 0x3F));
        return 2;
    } else if (cp <= 0xFFFF) {
        out[0] = (char)(0xE0 | ((cp >> 12) & 0x0F));
        out[1] = (char)(0x80 | ((cp >> 6) & 0x3F));
        out[2] = (char)(0x80 | (cp & 0x3F));
        return 3;
    } else if (cp <= 0x10FFFF) {
        out[0] = (char)(0xF0 | ((cp >> 18) & 0x07));
        out[1] = (char)(0x80 | ((cp >> 12) & 0x3F));
        out[2] = (char)(0x80 | ((cp >> 6) & 0x3F));
        out[3] = (char)(0x80 | (cp & 0x3F));
        return 4;
    }
    return -1;
}

static int urand_fd(void) {
    static int fd = -1;
    if (fd >= 0) return fd;
    fd = open("/dev/urandom", O_RDONLY);
    return fd;
}

static uint32_t rand_u32_secure(void) {
    int fd = urand_fd();
    if (fd >= 0) {
        uint32_t v = 0;
        ssize_t n = read(fd, &v, sizeof(v));
        if (n == (ssize_t)sizeof(v)) return v;
    }
    // Fallback (not cryptographically strong)
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    uint32_t seed = (uint32_t)(ts.tv_nsec ^ ts.tv_sec);
    seed ^= (uint32_t)getpid();
    seed ^= (uint32_t)((uintptr_t)&seed);
    // xorshift32
    seed ^= seed << 13; seed ^= seed >> 17; seed ^= seed << 5;
    return seed;
}

static uint32_t rand_between(uint32_t lo, uint32_t hi) {
    uint32_t span = hi - lo + 1u;
    uint32_t r = rand_u32_secure();
    return lo + (r % span);
}

static char* append_alloc(char* buf, size_t* cap, size_t* len, const char* data, size_t dlen) {
    if (*len + dlen + 1 > *cap) {
        size_t ncap = (*cap == 0) ? 128 : *cap;
        while (*len + dlen + 1 > ncap) ncap *= 2;
        char* nbuf = (char*)realloc(buf, ncap);
        if (!nbuf) {
            free(buf);
            return NULL;
        }
        buf = nbuf;
        *cap = ncap;
    }
    memcpy(buf + *len, data, dlen);
    *len += dlen;
    buf[*len] = '\0';
    return buf;
}

static char* append_fmt(char* buf, size_t* cap, size_t* len, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    char tmp[64];
    int n = vsnprintf(tmp, sizeof(tmp), fmt, ap);
    va_end(ap);
    if (n < 0) {
        free(buf);
        return NULL;
    }
    if ((size_t)n < sizeof(tmp)) {
        return append_alloc(buf, cap, len, tmp, (size_t)n);
    } else {
        size_t need = (size_t)n + 1;
        char* big = (char*)malloc(need);
        if (!big) {
            free(buf);
            return NULL;
        }
        va_start(ap, fmt);
        vsnprintf(big, need, fmt, ap);
        va_end(ap);
        buf = append_alloc(buf, cap, len, big, (size_t)n);
        free(big);
        return buf;
    }
}

static char* generate_random_unicode_string(size_t length) {
    char* buf = NULL;
    size_t cap = 0, len = 0;
    size_t produced = 0;
    size_t attempts = 0, maxAttempts = length * 50;
    while (produced < length && attempts < maxAttempts) {
        attempts++;
        const Range r = RANGES[rand_between(0, (uint32_t)RANGES_COUNT - 1)];
        uint32_t cp = rand_between(r.lo, r.hi);
        if (is_surrogate(cp)) continue;
        char bytes[4];
        int blen = utf8_encode_cp(cp, bytes);
        if (blen <= 0) continue;
        buf = append_alloc(buf, &cap, &len, bytes, (size_t)blen);
        if (!buf) return NULL;
        produced++;
    }
    if (produced < length) {
        free(buf);
        return NULL;
    }
    return buf ? buf : strdup("");
}

static char* to_codepoints_line(const char* utf8) {
    char* out = NULL;
    size_t cap = 0, len = 0;
    const unsigned char* s = (const unsigned char*)utf8;
    size_t i = 0, n = strlen(utf8);
    while (i < n) {
        uint32_t cp = 0;
        unsigned char c = s[i];
        size_t adv = 1;
        if (c <= 0x7F) {
            cp = c; adv = 1;
        } else if ((c & 0xE0) == 0xC0 && i + 1 < n) {
            cp = ((c & 0x1F) << 6) | (s[i+1] & 0x3F); adv = 2;
        } else if ((c & 0xF0) == 0xE0 && i + 2 < n) {
            cp = ((c & 0x0F) << 12) | ((s[i+1] & 0x3F) << 6) | (s[i+2] & 0x3F); adv = 3;
        } else if ((c & 0xF8) == 0xF0 && i + 3 < n) {
            cp = ((c & 0x07) << 18) | ((s[i+1] & 0x3F) << 12) | ((s[i+2] & 0x3F) << 6) | (s[i+3] & 0x3F); adv = 4;
        } else {
            i += 1;
            continue;
        }
        if (cp <= 0xFFFF) {
            out = append_fmt(out, &cap, &len, "U+%04X ", cp);
        } else {
            out = append_fmt(out, &cap, &len, "U+%06X ", cp);
        }
        if (!out) return NULL;
        i += adv;
    }
    if (out && len > 0 && out[len-1] == ' ') {
        out[len-1] = '\0';
    }
    return out ? out : strdup("");
}

char* create_temp_unicode_file(int length) {
    if (length < 1 || length > 1024) {
        return NULL;
    }

    char* original = generate_random_unicode_string((size_t)length);
    if (!original) return NULL;

    char* codepoints = to_codepoints_line(original);
    if (!codepoints) {
        free(original);
        return NULL;
    }

    const char* tmpdir = getenv("TMPDIR");
    if (!tmpdir || tmpdir[0] == '\0') tmpdir = "/tmp";
    char templ[PATH_MAX];
    int nw = snprintf(templ, sizeof(templ), "%s/%s", tmpdir, "task88_XXXXXX");
    if (nw < 0 || (size_t)nw >= sizeof(templ)) {
        free(original); free(codepoints);
        return NULL;
    }

    char pathbuf[PATH_MAX];
    strncpy(pathbuf, templ, sizeof(pathbuf)-1);
    pathbuf[sizeof(pathbuf)-1] = '\0';

    int fd = mkstemp(pathbuf); // creates a new file securely
    if (fd < 0) {
        free(original); free(codepoints);
        return NULL;
    }

    // Restrictive permissions 0600
    (void)fchmod(fd, S_IRUSR | S_IWUSR);

    // Build content
    char* content = NULL;
    size_t cap = 0, len = 0;
    content = append_alloc(content, &cap, &len, "Original: ", 10);
    if (!content) { close(fd); unlink(pathbuf); free(original); free(codepoints); return NULL; }
    content = append_alloc(content, &cap, &len, original, strlen(original));
    if (!content) { close(fd); unlink(pathbuf); free(original); free(codepoints); return NULL; }
    content = append_alloc(content, &cap, &len, "\nCodepoints: ", 13);
    if (!content) { close(fd); unlink(pathbuf); free(original); free(codepoints); return NULL; }
    content = append_alloc(content, &cap, &len, codepoints, strlen(codepoints));
    if (!content) { close(fd); unlink(pathbuf); free(original); free(codepoints); return NULL; }
    content = append_alloc(content, &cap, &len, "\n", 1);
    if (!content) { close(fd); unlink(pathbuf); free(original); free(codepoints); return NULL; }

    // Write content
    size_t total = len;
    size_t written = 0;
    while (written < total) {
        ssize_t w = write(fd, content + written, total - written);
        if (w < 0) {
            close(fd);
            unlink(pathbuf);
            free(original); free(codepoints); free(content);
            return NULL;
        }
        written += (size_t)w;
    }
    (void)fsync(fd);
    close(fd);

    free(original);
    free(codepoints);

    char* result = strdup(pathbuf);
    if (!result) {
        unlink(pathbuf);
        free(content);
        return NULL;
    }
    free(content);
    return result;
}

int main(void) {
    int tests[5] = {8, 16, 1, 32, 50};
    for (size_t i = 0; i < 5; i++) {
        char* path = create_temp_unicode_file(tests[i]);
        if (path) {
            printf("Created file (length %d): %s\n", tests[i], path);
            free(path);
        } else {
            printf("Error for length %d\n", tests[i]);
        }
    }
    return 0;
}