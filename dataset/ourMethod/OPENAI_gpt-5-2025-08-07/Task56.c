#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#else
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

static void secure_memzero(void* v, size_t n) {
    volatile unsigned char* p = (volatile unsigned char*)v;
    while (n--) {
        *p++ = 0;
    }
}

static int is_valid_user_id(const char* user_id) {
    if (user_id == NULL) return 0;
    size_t len = 0;
    for (const unsigned char* p = (const unsigned char*)user_id; *p; ++p) {
        if (*p < 33 || *p > 126) return 0; // printable non-space ASCII
        if (++len > 128) return 0;
    }
    return len > 0;
}

static int get_secure_random_bytes(uint8_t* buf, size_t len) {
    if (buf == NULL || len == 0) return 0;
#ifdef _WIN32
    NTSTATUS status = BCryptGenRandom(NULL, (PUCHAR)buf, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    return status == 0;
#else
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return 0;
    (void)fcntl(fd, F_SETFD, FD_CLOEXEC);

    size_t off = 0;
    while (off < len) {
        ssize_t r = read(fd, buf + off, len - off);
        if (r < 0) {
            if (errno == EINTR) continue;
            close(fd);
            return 0;
        }
        if (r == 0) {
            close(fd);
            return 0;
        }
        off += (size_t)r;
    }
    close(fd);
    return 1;
#endif
}

static char* base64url_encode(const uint8_t* data, size_t len) {
    static const char* alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    if (len == 0) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }

    size_t full = len / 3;
    size_t rem = len % 3;
    size_t out_len = full * 4 + (rem ? rem + 1 : 0);

    char* out = (char*)malloc(out_len + 1);
    if (!out) return NULL;

    size_t i = 0, o = 0;
    while (i + 3 <= len) {
        uint32_t v = ((uint32_t)data[i] << 16) | ((uint32_t)data[i + 1] << 8) | (uint32_t)data[i + 2];
        out[o++] = alphabet[(v >> 18) & 0x3F];
        out[o++] = alphabet[(v >> 12) & 0x3F];
        out[o++] = alphabet[(v >> 6) & 0x3F];
        out[o++] = alphabet[v & 0x3F];
        i += 3;
    }

    if (rem == 1) {
        uint32_t v = ((uint32_t)data[i] << 16);
        out[o++] = alphabet[(v >> 18) & 0x3F];
        out[o++] = alphabet[(v >> 12) & 0x3F];
    } else if (rem == 2) {
        uint32_t v = ((uint32_t)data[i] << 16) | ((uint32_t)data[i + 1] << 8);
        out[o++] = alphabet[(v >> 18) & 0x3F];
        out[o++] = alphabet[(v >> 12) & 0x3F];
        out[o++] = alphabet[(v >> 6) & 0x3F];
    }

    out[o] = '\0';
    return out;
}

char* generate_auth_token(const char* user_id, size_t entropy_bytes) {
    if (!is_valid_user_id(user_id)) return NULL;
    if (entropy_bytes < 16 || entropy_bytes > 64) return NULL;

    uint8_t* rnd = (uint8_t*)malloc(entropy_bytes);
    if (!rnd) return NULL;

    if (!get_secure_random_bytes(rnd, entropy_bytes)) {
        secure_memzero(rnd, entropy_bytes);
        free(rnd);
        return NULL;
    }

    char* token = base64url_encode(rnd, entropy_bytes);

    secure_memzero(rnd, entropy_bytes);
    free(rnd);

    return token; // caller must free
}

int main(void) {
    struct TestCase { const char* uid; size_t bytes; } tests[5] = {
        {"alice", 32},
        {"bob_01", 16},
        {"USER-3", 24},
        {"john.doe@example.com", 48},
        {"Zed-Admin_2025", 64}
    };

    char* tokens[5] = {0};
    size_t i;
    for (i = 0; i < 5; ++i) {
        tokens[i] = generate_auth_token(tests[i].uid, tests[i].bytes);
        if (tokens[i] == NULL) {
            printf("Test %zu: FAILED (generation error)\n", i + 1);
        } else {
            printf("Test %zu: OK (token length=%zu)\n", i + 1, strlen(tokens[i]));
        }
    }

    int unique = 1;
    for (i = 0; i < 5; ++i) {
        if (!tokens[i]) continue;
        for (size_t j = i + 1; j < 5; ++j) {
            if (!tokens[j]) continue;
            if (strcmp(tokens[i], tokens[j]) == 0) {
                unique = 0;
            }
        }
    }
    printf("Uniqueness check: %s\n", unique ? "PASSED" : "FAILED");

    for (i = 0; i < 5; ++i) {
        if (tokens[i]) {
            size_t len = strlen(tokens[i]);
            secure_memzero(tokens[i], len);
            free(tokens[i]);
            tokens[i] = NULL;
        }
    }
    return 0;
}