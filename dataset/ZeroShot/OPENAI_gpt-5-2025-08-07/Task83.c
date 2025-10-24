#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/hmac.h>

static int is_valid_aes_key(size_t n) {
    return n == 16 || n == 24 || n == 32;
}

static const EVP_CIPHER* cipher_for_key_size(size_t n) {
    if (n == 16) return EVP_aes_128_cbc();
    if (n == 24) return EVP_aes_192_cbc();
    if (n == 32) return EVP_aes_256_cbc();
    return NULL;
}

static unsigned char hex_value(char c) {
    if ('0' <= c && c <= '9') return (unsigned char)(c - '0');
    if ('a' <= c && c <= 'f') return (unsigned char)(c - 'a' + 10);
    if ('A' <= c && c <= 'F') return (unsigned char)(c - 'A' + 10);
    return 255;
}

unsigned char* hex_to_bytes(const char* s, size_t* out_len) {
    size_t len = strlen(s);
    if (len % 2 != 0) return NULL;
    size_t n = len / 2;
    unsigned char* out = (unsigned char*)malloc(n);
    if (!out) return NULL;
    for (size_t i = 0; i < n; i++) {
        unsigned char hi = hex_value(s[2*i]);
        unsigned char lo = hex_value(s[2*i + 1]);
        if (hi == 255 || lo == 255) {
            free(out);
            return NULL;
        }
        out[i] = (unsigned char)((hi << 4) | lo);
    }
    *out_len = n;
    return out;
}

void print_hex(const unsigned char* data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        printf("%02x", data[i]);
    }
}

unsigned char* encrypt_key(
    const unsigned char* enc_key, size_t enc_key_len,
    const unsigned char* mac_key, size_t mac_key_len,
    const unsigned char* key_to_encrypt, size_t key_to_encrypt_len,
    size_t* out_len
) {
    if (!is_valid_aes_key(enc_key_len)) {
        return NULL;
    }
    if (mac_key_len == 0 || !mac_key) {
        return NULL;
    }
    if (!key_to_encrypt) {
        return NULL;
    }

    unsigned char iv[16];
    if (RAND_bytes(iv, sizeof(iv)) != 1) {
        return NULL;
    }

    const EVP_CIPHER* cipher = cipher_for_key_size(enc_key_len);
    if (!cipher) return NULL;

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return NULL;

    int ok = 1;
    int len = 0, total = 0;
    int block = EVP_CIPHER_block_size(cipher);

    unsigned char* ciphertext = (unsigned char*)malloc(key_to_encrypt_len + block);
    if (!ciphertext) {
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }

    if (EVP_EncryptInit_ex(ctx, cipher, NULL, enc_key, iv) != 1) ok = 0;

    if (ok && EVP_EncryptUpdate(ctx, ciphertext, &len, key_to_encrypt, (int)key_to_encrypt_len) != 1) ok = 0;
    total = len;

    if (ok && EVP_EncryptFinal_ex(ctx, ciphertext + total, &len) != 1) ok = 0;
    total += len;

    EVP_CIPHER_CTX_free(ctx);

    if (!ok) {
        free(ciphertext);
        return NULL;
    }

    unsigned int tag_len = 0;
    unsigned char tag[EVP_MAX_MD_SIZE];
    HMAC_CTX* hctx = HMAC_CTX_new();
    if (!hctx) {
        free(ciphertext);
        return NULL;
    }
    if (HMAC_Init_ex(hctx, mac_key, (int)mac_key_len, EVP_sha256(), NULL) != 1 ||
        HMAC_Update(hctx, iv, sizeof(iv)) != 1 ||
        HMAC_Update(hctx, ciphertext, (size_t)total) != 1 ||
        HMAC_Final(hctx, tag, &tag_len) != 1) {
        HMAC_CTX_free(hctx);
        free(ciphertext);
        return NULL;
    }
    HMAC_CTX_free(hctx);

    size_t result_len = sizeof(iv) + (size_t)total + tag_len;
    unsigned char* result = (unsigned char*)malloc(result_len);
    if (!result) {
        free(ciphertext);
        return NULL;
    }
    memcpy(result, iv, sizeof(iv));
    memcpy(result + sizeof(iv), ciphertext, (size_t)total);
    memcpy(result + sizeof(iv) + (size_t)total, tag, tag_len);

    free(ciphertext);
    *out_len = result_len;
    return result;
}

int main(void) {
    OpenSSL_add_all_algorithms();

    const char* encKeyHex = "603deb1015ca71be2b73aef0857d7781f352c073b6108d72d9810a30914dff4";
    const char* macKeyHex = "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f";

    size_t enc_key_len = 0, mac_key_len = 0;
    unsigned char* enc_key = hex_to_bytes(encKeyHex, &enc_key_len);
    unsigned char* mac_key = hex_to_bytes(macKeyHex, &mac_key_len);

    const char* tests[] = {
        "00112233445566778899aabbccddeeff",
        "000102030405060708090a0b0c0d0e0f1011121314151617",
        "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f",
        "deadbeef",
        "aabbccddeeff"
    };

    for (int i = 0; i < 5; i++) {
        size_t pt_len = 0;
        unsigned char* pt = hex_to_bytes(tests[i], &pt_len);
        if (!pt) {
            fprintf(stderr, "Invalid test input.\n");
            continue;
        }
        size_t out_len = 0;
        unsigned char* out = encrypt_key(enc_key, enc_key_len, mac_key, mac_key_len, pt, pt_len, &out_len);
        if (!out) {
            fprintf(stderr, "Encryption failed for test %d.\n", i + 1);
            free(pt);
            continue;
        }
        printf("Test %d (iv|ciphertext|tag hex): ", i + 1);
        print_hex(out, out_len);
        printf("\n");
        free(pt);
        free(out);
    }

    free(enc_key);
    free(mac_key);
    return 0;
}