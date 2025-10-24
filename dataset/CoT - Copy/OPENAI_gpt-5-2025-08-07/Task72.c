// Requires OpenSSL development libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/aes.h>

// Static IV - for demonstration only; avoid static IVs in real systems.
static const unsigned char STATIC_IV[16] = {
    0x00, 0x11, 0x22, 0x33,
    0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xAA, 0xBB,
    0xCC, 0xDD, 0xEE, 0xFF
};

typedef struct {
    unsigned char* data;
    size_t len;
} EncryptResult;

static unsigned char* pad_pkcs7(const unsigned char* data, size_t data_len, size_t block_size, size_t* out_len) {
    if (block_size == 0) return NULL;
    size_t pad_len = block_size - (data_len % block_size);
    if (pad_len == 0) pad_len = block_size;
    *out_len = data_len + pad_len;
    unsigned char* out = (unsigned char*)malloc(*out_len);
    if (!out) return NULL;
    if (data_len > 0) memcpy(out, data, data_len);
    memset(out + data_len, (unsigned char)pad_len, pad_len);
    return out;
}

static const EVP_CIPHER* select_cipher(size_t key_len) {
    switch (key_len) {
        case 16: return EVP_aes_128_cbc();
        case 24: return EVP_aes_192_cbc();
        case 32: return EVP_aes_256_cbc();
        default: return NULL;
    }
}

EncryptResult encrypt_aes_cbc(const unsigned char* key, size_t key_len,
                              const unsigned char* plaintext, size_t plaintext_len) {
    EncryptResult res = {NULL, 0};

    const EVP_CIPHER* cipher = select_cipher(key_len);
    if (!cipher) {
        return res;
    }

    size_t padded_len = 0;
    unsigned char* padded = pad_pkcs7(plaintext, plaintext_len, 16, &padded_len);
    if (!padded) {
        return res;
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        free(padded);
        return res;
    }

    if (EVP_EncryptInit_ex(ctx, cipher, NULL, key, STATIC_IV) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(padded);
        return res;
    }

    // Disable internal padding; we already padded
    if (EVP_CIPHER_CTX_set_padding(ctx, 0) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(padded);
        return res;
    }

    int out_cap = (int)(padded_len + AES_BLOCK_SIZE);
    unsigned char* out = (unsigned char*)malloc(out_cap);
    if (!out) {
        EVP_CIPHER_CTX_free(ctx);
        free(padded);
        return res;
    }

    int out_len1 = 0, out_len2 = 0;
    if (EVP_EncryptUpdate(ctx, out, &out_len1, padded, (int)padded_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(padded);
        free(out);
        return res;
    }

    if (EVP_EncryptFinal_ex(ctx, out + out_len1, &out_len2) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(padded);
        free(out);
        return res;
    }

    res.data = out;
    res.len = (size_t)(out_len1 + out_len2);

    EVP_CIPHER_CTX_free(ctx);
    free(padded);
    return res;
}

static void print_hex(const unsigned char* data, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

int main(void) {
    // Test case 1
    unsigned char key1[16];
    for (int i = 0; i < 16; ++i) key1[i] = (unsigned char)i;
    unsigned char pt1[] = { };
    EncryptResult r1 = encrypt_aes_cbc(key1, sizeof(key1), pt1, 0);
    printf("TC1: "); print_hex(r1.data, r1.len);
    free(r1.data);

    // Test case 2
    unsigned char key2[16];
    memset(key2, 0x01, sizeof(key2));
    unsigned char pt2[] = { 'A' };
    EncryptResult r2 = encrypt_aes_cbc(key2, sizeof(key2), pt2, sizeof(pt2));
    printf("TC2: "); print_hex(r2.data, r2.len);
    free(r2.data);

    // Test case 3
    unsigned char key3[16] = "Sixteen byte key"; // 16 bytes
    unsigned char pt3[] = "Hello World";
    EncryptResult r3 = encrypt_aes_cbc(key3, sizeof(key3), pt3, strlen((char*)pt3));
    printf("TC3: "); print_hex(r3.data, r3.len);
    free(r3.data);

    // Test case 4 (AES-192)
    unsigned char key4[24];
    for (int i = 0; i < 24; ++i) key4[i] = (unsigned char)i;
    const char* s4 = "The quick brown fox jumps over the lazy dog";
    EncryptResult r4 = encrypt_aes_cbc(key4, sizeof(key4), (const unsigned char*)s4, strlen(s4));
    printf("TC4: "); print_hex(r4.data, r4.len);
    free(r4.data);

    // Test case 5 (AES-256)
    unsigned char key5[32];
    for (int i = 0; i < 32; ++i) key5[i] = (unsigned char)i;
    const char* s5 = "Data with a length not multiple of block size.";
    EncryptResult r5 = encrypt_aes_cbc(key5, sizeof(key5), (const unsigned char*)s5, strlen(s5));
    printf("TC5: "); print_hex(r5.data, r5.len);
    free(r5.data);

    return 0;
}