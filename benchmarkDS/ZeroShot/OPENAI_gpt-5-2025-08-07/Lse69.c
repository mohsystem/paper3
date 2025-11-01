#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#include <sys/types.h>
#define MKDIR(path) mkdir(path, 0700)
#endif

#define MAX_SIZE (10 * 1024 * 1024) /* 10 MB */
#define BASE_DIR "out_c"

static int is_safe_filename(const char* name) {
    if (!name) return 0;
    size_t len = strlen(name);
    if (len == 0 || len > 255) return 0;
    if ((len == 1 && name[0] == '.') || (len == 2 && name[0] == '.' && name[1] == '.')) return 0;
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)name[i];
        int ok = (c >= 'A' && c <= 'Z') ||
                 (c >= 'a' && c <= 'z') ||
                 (c >= '0' && c <= '9') ||
                 c == '.' || c == '_' || c == '-';
        if (!ok) return 0;
    }
    return 1;
}

static char* base64_encode(const unsigned char* data, size_t in_len, size_t* out_len) {
    static const char tbl[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    if (in_len == 0) {
        char* out = (char*)malloc(1);
        if (!out) return NULL;
        out[0] = '\0';
        if (out_len) *out_len = 0;
        return out;
    }
    if (in_len > MAX_SIZE) return NULL; /* enforce limit to avoid overflow */
    size_t olen = ((in_len + 2) / 3) * 4;
    char* out = (char*)malloc(olen + 1);
    if (!out) return NULL;

    size_t i = 0, j = 0;
    while (i + 2 < in_len) {
        unsigned int n = (data[i] << 16) | (data[i + 1] << 8) | (data[i + 2]);
        out[j++] = tbl[(n >> 18) & 63];
        out[j++] = tbl[(n >> 12) & 63];
        out[j++] = tbl[(n >> 6) & 63];
        out[j++] = tbl[n & 63];
        i += 3;
    }
    if (i < in_len) {
        unsigned int n = data[i] << 16;
        if ((i + 1) < in_len) n |= (data[i + 1] << 8);
        out[j++] = tbl[(n >> 18) & 63];
        out[j++] = tbl[(n >> 12) & 63];
        if ((i + 1) < in_len) {
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

/* Returns number of bytes written to the encoded file; 0 on error */
size_t upload_image(const unsigned char* imageData, size_t imageLen, const char* outputFileName) {
    if (!imageData && imageLen != 0) {
        errno = EINVAL;
        return 0;
    }
    if (imageLen > MAX_SIZE) {
        errno = EFBIG;
        return 0;
    }
    if (!is_safe_filename(outputFileName)) {
        errno = EINVAL;
        return 0;
    }

    /* Create base directory if it doesn't exist */
    MKDIR(BASE_DIR);

    char path[512];
    int n = snprintf(path, sizeof(path), "%s/%s", BASE_DIR, outputFileName);
    if (n <= 0 || (size_t)n >= sizeof(path)) {
        errno = ENAMETOOLONG;
        return 0;
    }

    /* Prevent overwrite (best-effort) */
    FILE* chk = fopen(path, "rb");
    if (chk) {
        fclose(chk);
        errno = EEXIST;
        return 0;
    }

    size_t enc_len = 0;
    char* encoded = base64_encode(imageData, imageLen, &enc_len);
    if (!encoded && imageLen != 0) {
        errno = ENOMEM;
        return 0;
    }

    FILE* f = fopen(path, "wb");
    if (!f) {
        free(encoded);
        return 0;
    }

    size_t written = 0;
    if (enc_len > 0) {
        written = fwrite(encoded, 1, enc_len, f);
    }
    fclose(f);
    free(encoded);

    if (written != enc_len) {
        errno = EIO;
        return 0;
    }
    return enc_len;
}

static void fill_bytes(unsigned char* buf, size_t n, unsigned int seed) {
    /* deterministic filler to avoid insecure RNG usage */
    unsigned int x = seed ? seed : 0xA5A5A5A5u;
    for (size_t i = 0; i < n; ++i) {
        x ^= x << 13; x ^= x >> 17; x ^= x << 5; /* xorshift */
        buf[i] = (unsigned char)(x & 0xFF);
    }
}

int main(void) {
    unsigned char *b0 = NULL;
    unsigned char b1[10]; fill_bytes(b1, sizeof(b1), 1u);
    unsigned char b2[1024]; fill_bytes(b2, sizeof(b2), 2u);
    unsigned char b3[2048]; fill_bytes(b3, sizeof(b3), 3u);
    unsigned char b4[9999]; fill_bytes(b4, sizeof(b4), 4u);

    size_t r;

    r = upload_image(b0, 0, "img0.b64");
    if (r == 0 && errno != 0) printf("Test1 failed: %d\n", errno); else printf("Test1: %zu bytes written.\n", r);

    r = upload_image(b1, sizeof(b1), "img1.b64");
    if (r == 0 && errno != 0) printf("Test2 failed: %d\n", errno); else printf("Test2: %zu bytes written.\n", r);

    r = upload_image(b2, sizeof(b2), "img2.b64");
    if (r == 0 && errno != 0) printf("Test3 failed: %d\n", errno); else printf("Test3: %zu bytes written.\n", r);

    r = upload_image(b3, sizeof(b3), "img3.b64");
    if (r == 0 && errno != 0) printf("Test4 failed: %d\n", errno); else printf("Test4: %zu bytes written.\n", r);

    r = upload_image(b4, sizeof(b4), "img4.b64");
    if (r == 0 && errno != 0) printf("Test5 failed: %d\n", errno); else printf("Test5: %zu bytes written.\n", r);

    return 0;
}