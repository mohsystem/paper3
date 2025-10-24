#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <openssl/evp.h>

static unsigned char* hex_to_bytes(const char* s, size_t* out_len) {
    size_t len = strlen(s);
    if (len % 2 != 0) return NULL;
    size_t n = len / 2;
    unsigned char* out = (unsigned char*)malloc(n);
    if (!out) return NULL;
    for (size_t i = 0; i < n; ++i) {
        char c1 = s[2*i], c2 = s[2*i+1];
        int v1 = (c1 >= '0' && c1 <= '9') ? c1 - '0' :
                 (tolower((unsigned char)c1) >= 'a' && tolower((unsigned char)c1) <= 'f') ? tolower((unsigned char)c1) - 'a' + 10 : -1;
        int v2 = (c2 >= '0' && c2 <= '9') ? c2 - '0' :
                 (tolower((unsigned char)c2) >= 'a' && tolower((unsigned char)c2) <= 'f') ? tolower((unsigned char)c2) - 'a' + 10 : -1;
        if (v1 < 0 || v2 < 0) { free(out); return NULL; }
        out[i] = (unsigned char)((v1 << 4) | v2);
    }
    *out_len = n;
    return out;
}

static char* bytes_to_hex(const unsigned char* data, size_t len) {
    static const char* hex = "0123456789abcdef";
    char* out = (char*)malloc(len * 2 + 1);
    if (!out) return NULL;
    for (size_t i = 0; i < len; ++i) {
        out[2*i] = hex[data[i] >> 4];
        out[2*i + 1] = hex[data[i] & 0x0F];
    }
    out[len * 2] = '\0';
    return out;
}

unsigned char* encrypt_key_aes_cbc(const unsigned char* keyToEncrypt, int keyToEncryptLen,
                                   const unsigned char* cipherKey, int cipherKeyLen,
                                   const unsigned char* iv, int ivLen,
                                   int* outLen) {
    if (!(cipherKeyLen == 16 || cipherKeyLen == 24 || cipherKeyLen == 32)) {
        return NULL;
    }
    if (ivLen != 16) {
        return NULL;
    }

    const EVP_CIPHER* cipher = NULL;
    if (cipherKeyLen == 16) cipher = EVP_aes_128_cbc();
    else if (cipherKeyLen == 24) cipher = EVP_aes_192_cbc();
    else if (cipherKeyLen == 32) cipher = EVP_aes_256_cbc();

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return NULL;

    unsigned char* out = (unsigned char*)malloc(keyToEncryptLen + EVP_CIPHER_block_size(cipher));
    if (!out) {
        EVP_CIPHER_CTX_free(ctx);
        return NULL;
    }

    int len1 = 0, len2 = 0;
    if (EVP_EncryptInit_ex(ctx, cipher, NULL, cipherKey, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(out);
        return NULL;
    }
    if (EVP_EncryptUpdate(ctx, out, &len1, keyToEncrypt, keyToEncryptLen) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(out);
        return NULL;
    }
    if (EVP_EncryptFinal_ex(ctx, out + len1, &len2) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        free(out);
        return NULL;
    }
    *outLen = len1 + len2;
    EVP_CIPHER_CTX_free(ctx);
    return out;
}

int main(void) {
    // Test case 1
    size_t k1len, ck1len, iv1len;
    unsigned char* k1 = hex_to_bytes("00112233445566778899aabbccddeeff", &k1len);
    unsigned char* ck1 = hex_to_bytes("000102030405060708090a0b0c0d0e0f", &ck1len);
    unsigned char* iv1 = hex_to_bytes("101112131415161718191a1b1c1d1e1f", &iv1len);
    int ct1len = 0;
    unsigned char* ct1 = encrypt_key_aes_cbc(k1, (int)k1len, ck1, (int)ck1len, iv1, (int)iv1len, &ct1len);
    char* h1 = bytes_to_hex(ct1, (size_t)ct1len);
    puts(h1);

    // Test case 2
    size_t k2len, ck2len, iv2len;
    unsigned char* k2 = hex_to_bytes("000102030405060708090a0b0c0d0e0f1011121314151617", &k2len);
    unsigned char* ck2 = hex_to_bytes("2b7e151628aed2a6abf7158809cf4f3c", &ck2len);
    unsigned char* iv2 = hex_to_bytes("000102030405060708090a0b0c0d0e0f", &iv2len);
    int ct2len = 0;
    unsigned char* ct2 = encrypt_key_aes_cbc(k2, (int)k2len, ck2, (int)ck2len, iv2, (int)iv2len, &ct2len);
    char* h2 = bytes_to_hex(ct2, (size_t)ct2len);
    puts(h2);

    // Test case 3
    size_t k3len, ck3len, iv3len;
    unsigned char* k3 = hex_to_bytes("000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f", &k3len);
    unsigned char* ck3 = hex_to_bytes("603deb1015ca71be2b73aef0857d77811f352c073b6108d72d9810a30914dff4", &ck3len);
    unsigned char* iv3 = hex_to_bytes("0f0e0d0c0b0a09080706050403020100", &iv3len);
    int ct3len = 0;
    unsigned char* ct3 = encrypt_key_aes_cbc(k3, (int)k3len, ck3, (int)ck3len, iv3, (int)iv3len, &ct3len);
    char* h3 = bytes_to_hex(ct3, (size_t)ct3len);
    puts(h3);

    // Test case 4
    size_t k4len, ck4len, iv4len;
    unsigned char* k4 = hex_to_bytes("00112233445566778899aabbccddeeff00112233", &k4len);
    unsigned char* ck4 = hex_to_bytes("8e73b0f7da0e6452c810f32b809079e562f8ead2522c6b7b", &ck4len);
    unsigned char* iv4 = hex_to_bytes("1f1e1d1c1b1a19181716151413121110", &iv4len);
    int ct4len = 0;
    unsigned char* ct4 = encrypt_key_aes_cbc(k4, (int)k4len, ck4, (int)ck4len, iv4, (int)iv4len, &ct4len);
    char* h4 = bytes_to_hex(ct4, (size_t)ct4len);
    puts(h4);

    // Test case 5
    size_t k5len, ck5len, iv5len;
    unsigned char* k5 = hex_to_bytes("000102030405060708090a0b0c0d0e", &k5len);
    unsigned char* ck5 = hex_to_bytes("000102030405060708090a0b0c0d0e0f", &ck5len);
    unsigned char* iv5 = hex_to_bytes("aabbccddeeff00112233445566778899", &iv5len);
    int ct5len = 0;
    unsigned char* ct5 = encrypt_key_aes_cbc(k5, (int)k5len, ck5, (int)ck5len, iv5, (int)iv5len, &ct5len);
    char* h5 = bytes_to_hex(ct5, (size_t)ct5len);
    puts(h5);

    // Cleanup
    free(k1); free(ck1); free(iv1); free(ct1); free(h1);
    free(k2); free(ck2); free(iv2); free(ct2); free(h2);
    free(k3); free(ck3); free(iv3); free(ct3); free(h3);
    free(k4); free(ck4); free(iv4); free(ct4); free(h4);
    free(k5); free(ck5); free(iv5); free(ct5); free(h5);

    return 0;
}