// WARNING: Using a static IV is insecure and only for demonstration/testing.
#include <openssl/evp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const unsigned char STATIC_IV[16] = "0123456789ABCDEF";

unsigned char* pad_pkcs7(const unsigned char* data, size_t len, size_t block_size, size_t* out_len) {
    size_t padding = block_size - (len % block_size);
    if (padding == 0) padding = block_size;
    *out_len = len + padding;
    unsigned char* out = (unsigned char*)malloc(*out_len);
    if (!out) return NULL;
    memcpy(out, data, len);
    memset(out + len, (unsigned char)padding, padding);
    return out;
}

const EVP_CIPHER* cipher_for_key_size(size_t key_len) {
    switch (key_len) {
        case 16: return EVP_aes_128_cbc();
        case 24: return EVP_aes_192_cbc();
        case 32: return EVP_aes_256_cbc();
        default: return NULL;
    }
}

int encrypt_aes_cbc(const unsigned char* key, size_t key_len,
                    const unsigned char* data, size_t data_len,
                    unsigned char** out, size_t* out_len) {
    const EVP_CIPHER* cipher = cipher_for_key_size(key_len);
    if (!cipher) return 0;

    size_t padded_len = 0;
    unsigned char* padded = pad_pkcs7(data, data_len, 16, &padded_len);
    if (!padded) return 0;

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) { free(padded); return 0; }

    int success = 0;
    int outl1 = 0, outl2 = 0;
    unsigned char* outbuf = (unsigned char*)malloc(padded_len + 16);
    if (!outbuf) { EVP_CIPHER_CTX_free(ctx); free(padded); return 0; }

    if (EVP_EncryptInit_ex(ctx, cipher, NULL, key, STATIC_IV) != 1) goto cleanup;
    EVP_CIPHER_CTX_set_padding(ctx, 0);

    if (EVP_EncryptUpdate(ctx, outbuf, &outl1, padded, (int)padded_len) != 1) goto cleanup;
    if (EVP_EncryptFinal_ex(ctx, outbuf + outl1, &outl2) != 1) goto cleanup;

    *out_len = (size_t)(outl1 + outl2);
    *out = outbuf;
    outbuf = NULL;
    success = 1;

cleanup:
    if (outbuf) free(outbuf);
    EVP_CIPHER_CTX_free(ctx);
    free(padded);
    return success;
}

void print_hex(const unsigned char* data, size_t len) {
    for (size_t i = 0; i < len; ++i) printf("%02x", data[i]);
    printf("\n");
}

int main(void) {
    const unsigned char key[16] = "ThisIsA16ByteKey"; // 16-byte key (AES-128)

    const char* tests[5] = {
        "",
        "Hello, AES-CBC!",
        "The quick brown fox jumps over the lazy dog",
        "1234567890ABCDEF1234",
        "Data with \x00 nulls \x00 inside"
    };

    for (int i = 0; i < 5; ++i) {
        const unsigned char* pt = (const unsigned char*)tests[i];
        size_t pt_len = strlen(tests[i]); // safe here as input strings contain embedded nulls only for display; strlen stops at first null
        // For accurate length with potential embedded nulls, you'd pass explicit lengths.
        unsigned char* ct = NULL;
        size_t ct_len = 0;
        if (!encrypt_aes_cbc(key, sizeof(key), pt, pt_len, &ct, &ct_len)) {
            fprintf(stderr, "Encryption failed for test %d\n", i + 1);
            continue;
        }
        printf("Test %d Hex:    ", i + 1);
        print_hex(ct, ct_len);
        free(ct);
    }
    return 0;
}