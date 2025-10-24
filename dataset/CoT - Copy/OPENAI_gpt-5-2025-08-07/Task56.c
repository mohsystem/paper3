/* Chain-of-Through process in code generation:
 * 1) Problem understanding: Create a cryptographically secure, URL-safe authentication token.
 * 2) Security requirements: Strong RNG from OS, sufficient entropy (>=128 bits), URL-safe Base64 without padding.
 * 3) Secure coding generation: Validate inputs, robust OS RNG (/dev/urandom or BCryptGenRandom), careful memory management.
 * 4) Code review: Ensured strong RNG, safe encoding, error checks, and no undefined behavior.
 * 5) Secure code output: Final code uses CSPRNG and safe encoding with input validation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifdef _WIN32
  #define NOMINMAX
  #include <windows.h>
  #include <bcrypt.h>
  #pragma comment(lib, "Bcrypt.lib")
#else
  #include <fcntl.h>
  #include <unistd.h>
  #include <errno.h>
#endif

static int get_secure_random_bytes(uint8_t* buf, size_t len) {
#ifdef _WIN32
    NTSTATUS status = BCryptGenRandom(NULL, buf, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    return status == 0 ? 1 : 0;
#else
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return 0;
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
    static const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    // Max output length without padding is ceil(len/3)*4, minus up to 2 if remainder 1 or 2.
    size_t full_groups = len / 3;
    size_t rem = len % 3;

    size_t out_len = full_groups * 4;
    if (rem == 1) out_len += 2;
    else if (rem == 2) out_len += 3;

    char* out = (char*)malloc(out_len + 1);
    if (!out) return NULL;

    size_t i = 0, j = 0;
    while (i + 3 <= len) {
        uint32_t triple = ((uint32_t)data[i] << 16) | ((uint32_t)data[i + 1] << 8) | (uint32_t)data[i + 2];
        out[j++] = alphabet[(triple >> 18) & 0x3F];
        out[j++] = alphabet[(triple >> 12) & 0x3F];
        out[j++] = alphabet[(triple >> 6) & 0x3F];
        out[j++] = alphabet[triple & 0x3F];
        i += 3;
    }

    if (rem == 1) {
        uint8_t b0 = data[i];
        out[j++] = alphabet[(b0 >> 2) & 0x3F];
        out[j++] = alphabet[(b0 & 0x03) << 4];
    } else if (rem == 2) {
        uint8_t b0 = data[i];
        uint8_t b1 = data[i + 1];
        out[j++] = alphabet[(b0 >> 2) & 0x3F];
        out[j++] = alphabet[((b0 & 0x03) << 4) | (b1 >> 4)];
        out[j++] = alphabet[(b1 & 0x0F) << 2];
    }

    out[j] = '\0';
    return out;
}

char* generate_auth_token(size_t num_bytes) {
    if (num_bytes < 16 || num_bytes > 64) {
        return NULL;
    }
    uint8_t* buf = (uint8_t*)malloc(num_bytes);
    if (!buf) return NULL;

    if (!get_secure_random_bytes(buf, num_bytes)) {
        free(buf);
        return NULL;
    }

    char* token = base64url_encode(buf, num_bytes);
    free(buf);
    return token; // caller must free
}

int main(void) {
    size_t tests[5] = {16, 24, 32, 48, 64};
    for (int i = 0; i < 5; ++i) {
        char* t = generate_auth_token(tests[i]);
        if (!t) {
            fprintf(stderr, "Failed to generate token for %zu bytes\n", tests[i]);
            return 1;
        }
        printf("Bytes=%zu Token=%s Length=%zu\n", tests[i], t, strlen(t));
        free(t);
    }
    return 0;
}