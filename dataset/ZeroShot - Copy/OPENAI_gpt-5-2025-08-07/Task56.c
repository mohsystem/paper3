#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#if defined(_WIN32)
  #include <windows.h>
  #include <bcrypt.h>
  #pragma comment(lib, "bcrypt.lib")
#else
  #include <fcntl.h>
  #include <unistd.h>
  #include <errno.h>
#endif

static int secure_random_bytes(uint8_t* out, size_t len) {
    if (len == 0) return 0;
#if defined(_WIN32)
    NTSTATUS status = BCryptGenRandom(NULL, out, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    return status == 0 ? 0 : -1;
#else
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return -1;
    size_t total = 0;
    while (total < len) {
        ssize_t r = read(fd, out + total, len - total);
        if (r < 0) {
            int err = errno;
            (void)err;
            close(fd);
            return -1;
        }
        if (r == 0) {
            close(fd);
            return -1;
        }
        total += (size_t)r;
    }
    close(fd);
    return 0;
#endif
}

static char* base64_url_encode(const uint8_t* data, size_t len) {
    static const char* tbl = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t b64_len = 4 * ((len + 2) / 3);
    char* tmp = (char*)malloc(b64_len + 1);
    if (!tmp) return NULL;

    size_t i = 0, j = 0;
    while (i + 3 <= len) {
        uint32_t b = ((uint32_t)data[i] << 16) | ((uint32_t)data[i+1] << 8) | (uint32_t)data[i+2];
        tmp[j++] = tbl[(b >> 18) & 0x3F];
        tmp[j++] = tbl[(b >> 12) & 0x3F];
        tmp[j++] = tbl[(b >> 6) & 0x3F];
        tmp[j++] = tbl[b & 0x3F];
        i += 3;
    }
    if (i + 1 == len) {
        uint32_t b = ((uint32_t)data[i] << 16);
        tmp[j++] = tbl[(b >> 18) & 0x3F];
        tmp[j++] = tbl[(b >> 12) & 0x3F];
        tmp[j++] = '=';
        tmp[j++] = '=';
    } else if (i + 2 == len) {
        uint32_t b = ((uint32_t)data[i] << 16) | ((uint32_t)data[i+1] << 8);
        tmp[j++] = tbl[(b >> 18) & 0x3F];
        tmp[j++] = tbl[(b >> 12) & 0x3F];
        tmp[j++] = tbl[(b >> 6) & 0x3F];
        tmp[j++] = '=';
    }
    tmp[j] = '\0';

    // Convert to URL-safe and remove '='
    for (size_t k = 0; k < j; ++k) {
        if (tmp[k] == '+') tmp[k] = '-';
        else if (tmp[k] == '/') tmp[k] = '_';
    }

    // Remove padding '='
    size_t end = j;
    while (end > 0 && tmp[end - 1] == '=') {
        --end;
    }

    char* out = (char*)malloc(end + 1);
    if (!out) {
        free(tmp);
        return NULL;
    }
    memcpy(out, tmp, end);
    out[end] = '\0';
    free(tmp);
    return out;
}

// Generates a URL-safe base64 token with cryptographically secure randomness.
// Ensures at least 32 bytes of entropy.
// Returns a heap-allocated C string that the caller must free, or NULL on error.
char* generate_token(size_t num_bytes) {
    size_t n = num_bytes < 32 ? 32 : num_bytes;
    uint8_t* buf = (uint8_t*)malloc(n);
    if (!buf) return NULL;
    if (secure_random_bytes(buf, n) != 0) {
        free(buf);
        return NULL;
    }
    char* encoded = base64_url_encode(buf, n);
    free(buf);
    return encoded;
}

int main(void) {
    size_t tests[5] = {32, 48, 64, 16, 0};
    for (int i = 0; i < 5; ++i) {
        char* tok = generate_token(tests[i]);
        if (tok) {
            printf("C Token %d (%zu bytes): %s\n", i + 1, tests[i] < 32 ? (size_t)32 : tests[i], tok);
            free(tok);
        } else {
            printf("C Token %d: generation failed\n", i + 1);
        }
    }
    return 0;
}