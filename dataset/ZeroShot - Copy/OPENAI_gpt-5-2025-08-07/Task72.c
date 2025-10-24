#include <openssl/evp.h>
#include <openssl/err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const unsigned char STATIC_IV[16] = {
    0x00,0x01,0x02,0x03,
    0x04,0x05,0x06,0x07,
    0x08,0x09,0x0A,0x0B,
    0x0C,0x0D,0x0E,0x0F
};
static const size_t BLOCK_SIZE = 16;

static unsigned char* pkcs7_pad(const unsigned char* data, size_t len, size_t block_size, size_t* out_len) {
    if (block_size == 0) return NULL;
    size_t pad_len = block_size - (len % block_size);
    if (pad_len == 0) pad_len = block_size;
    *out_len = len + pad_len;
    unsigned char* out = (unsigned char*)malloc(*out_len);
    if (!out) return NULL;
    if (len > 0) memcpy(out, data, len);
    memset(out + len, (unsigned char)pad_len, pad_len);
    return out;
}

static int encrypt(const unsigned char* key, size_t key_len,
                   const unsigned char* data, size_t data_len,
                   unsigned char** out, size_t* out_len) {
    *out = NULL;
    *out_len = 0;

    if (!(key_len == 16 || key_len == 24 || key_len == 32)) {
        return 0;
    }

    const EVP_CIPHER* cipher = NULL;
    switch (key_len) {
        case 16: cipher = EVP_aes_128_cbc(); break;
        case 24: cipher = EVP_aes_192_cbc(); break;
        case 32: cipher = EVP_aes_256_cbc(); break;
        default: return 0;
    }

    size_t padded_len = 0;
    unsigned char* padded = pkcs7_pad(data, data_len, BLOCK_SIZE, &padded_len);
    if (!padded) return 0;

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) { free(padded); return 0; }

    int ok = 0;
    int outl1 = 0, outl2 = 0;
    unsigned char* buf = (unsigned char*)malloc(padded_len + BLOCK_SIZE);
    if (!buf) { EVP_CIPHER_CTX_free(ctx); free(padded); return 0; }

    if (EVP_EncryptInit_ex(ctx, cipher, NULL, key, STATIC_IV) != 1) goto cleanup;
    if (EVP_CIPHER_CTX_set_padding(ctx, 0) != 1) goto cleanup;
    if (EVP_EncryptUpdate(ctx, buf, &outl1, padded, (int)padded_len) != 1) goto cleanup;
    if (EVP_EncryptFinal_ex(ctx, buf + outl1, &outl2) != 1) goto cleanup;

    *out_len = (size_t)(outl1 + outl2);
    *out = (unsigned char*)malloc(*out_len);
    if (!*out) goto cleanup;
    memcpy(*out, buf, *out_len);
    ok = 1;

cleanup:
    if (!ok) {
        if (*out) { free(*out); *out = NULL; }
        *out_len = 0;
    }
    if (buf) free(buf);
    EVP_CIPHER_CTX_free(ctx);
    free(padded);
    return ok;
}

static char* b64encode(const unsigned char* data, size_t len) {
    static const char tbl[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t out_len = 4 * ((len + 2) / 3);
    char* out = (char*)malloc(out_len + 1);
    if (!out) return NULL;

    size_t i = 0, j = 0;
    while (i + 2 < len) {
        unsigned int n = (data[i] << 16) | (data[i+1] << 8) | data[i+2];
        out[j++] = tbl[(n >> 18) & 63];
        out[j++] = tbl[(n >> 12) & 63];
        out[j++] = tbl[(n >> 6) & 63];
        out[j++] = tbl[n & 63];
        i += 3;
    }
    if (i < len) {
        unsigned int n = data[i] << 16;
        out[j++] = tbl[(n >> 18) & 63];
        if (i + 1 < len) {
            n |= data[i+1] << 8;
            out[j++] = tbl[(n >> 12) & 63];
            out[j++] = tbl[(n >> 6) & 63];
            out[j++] = '=';
        } else {
            out[j++] = tbl[(n >> 12) & 63];
            out[j++] = '=';
            out[j++] = '=';
        }
    }
    out[j] = '\0';
    return out;
}

int main(void) {
    const unsigned char key[16] = "0123456789ABCDEF"; // 16-byte key
    const char* tests[5] = {
        "",
        "Hello, World!",
        "The quick brown fox jumps over the lazy dog",
        "1234567890abcdef",
        "A longer message that will span multiple blocks to test padding and encryption correctness."
    };

    for (int i = 0; i < 5; i++) {
        const unsigned char* msg = (const unsigned char*)tests[i];
        size_t msg_len = strlen(tests[i]);
        unsigned char* ct = NULL;
        size_t ct_len = 0;
        if (!encrypt(key, sizeof(key), msg, msg_len, &ct, &ct_len)) {
            fprintf(stderr, "Encryption error on test %d\n", i + 1);
            continue;
        }
        char* b64 = b64encode(ct, ct_len);
        if (b64) {
            printf("Test %d: %s\n", i + 1, b64);
            free(b64);
        }
        free(ct);
    }
    return 0;
}