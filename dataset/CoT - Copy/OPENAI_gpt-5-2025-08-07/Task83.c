/* Chain-of-Through secure implementation for AES-CBC encryption.
 * 1) Problem understanding: Encrypt provided key material using AES-CBC and return Base64(IV||ciphertext).
 * 2) Security requirements: Use random IV per encryption, validate AES key, PKCS7 padding via EVP.
 * 3) Secure coding generation: Use OpenSSL EVP API with robust error checks.
 * 4) Code review: Ensure frees, handle errors, avoid static IV.
 * 5) Secure code output: Provide 5 test cases.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

/* Base64 encode using OpenSSL BIO, returns malloc'ed string (caller must free) */
static char* base64_encode_buf(const unsigned char* data, size_t len) {
    char* out = NULL;
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO* mem = BIO_new(BIO_s_mem());
    if (!b64 || !mem) {
        if (b64) BIO_free(b64);
        if (mem) BIO_free(mem);
        return NULL;
    }
    b64 = BIO_push(b64, mem);
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    if (BIO_write(b64, data, (int)len) <= 0) {
        BIO_free_all(b64);
        return NULL;
    }
    if (BIO_flush(b64) != 1) {
        BIO_free_all(b64);
        return NULL;
    }
    BUF_MEM* mem_ptr = NULL;
    BIO_get_mem_ptr(b64, &mem_ptr);
    if (!mem_ptr || !mem_ptr->data || mem_ptr->length == 0) {
        BIO_free_all(b64);
        return NULL;
    }
    out = (char*)malloc(mem_ptr->length + 1);
    if (!out) {
        BIO_free_all(b64);
        return NULL;
    }
    memcpy(out, mem_ptr->data, mem_ptr->length);
    out[mem_ptr->length] = '\0';
    BIO_free_all(b64);
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

/* Encrypts key_to_encrypt using AES-CBC with aes_key. Returns Base64(IV||ciphertext) string (malloc'ed), or NULL on error. */
char* encrypt_key(const uint8_t* aes_key, size_t aes_key_len, const uint8_t* key_to_encrypt, size_t key_to_encrypt_len) {
    if (!aes_key || !key_to_encrypt || key_to_encrypt_len == 0) return NULL;
    const EVP_CIPHER* cipher = select_cipher(aes_key_len);
    if (!cipher) return NULL;

    uint8_t iv[16];
    if (RAND_bytes(iv, sizeof(iv)) != 1) return NULL;

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return NULL;

    int ok = 1;
    int outlen1 = 0, outlen2 = 0;
    int block = EVP_CIPHER_block_size(cipher);
    size_t max_ct_len = key_to_encrypt_len + (size_t)block;
    unsigned char* ct = (unsigned char*)malloc(max_ct_len);
    if (!ct) { EVP_CIPHER_CTX_free(ctx); return NULL; }

    if (EVP_EncryptInit_ex(ctx, cipher, NULL, aes_key, iv) != 1) ok = 0;
    if (ok && EVP_EncryptUpdate(ctx, ct, &outlen1, key_to_encrypt, (int)key_to_encrypt_len) != 1) ok = 0;
    if (ok && EVP_EncryptFinal_ex(ctx, ct + outlen1, &outlen2) != 1) ok = 0;

    char* b64 = NULL;
    if (ok) {
        size_t total_ct = (size_t)(outlen1 + outlen2);
        unsigned char* out = (unsigned char*)malloc(sizeof(iv) + total_ct);
        if (out) {
            memcpy(out, iv, sizeof(iv));
            memcpy(out + sizeof(iv), ct, total_ct);
            b64 = base64_encode_buf(out, sizeof(iv) + total_ct);
            free(out);
        }
    }

    free(ct);
    EVP_CIPHER_CTX_free(ctx);
    return b64;
}

/* Helper: hex to bytes (malloc'ed) */
static uint8_t* hexToBytes(const char* hex, size_t* out_len) {
    size_t len = strlen(hex);
    if (len % 2 != 0) return NULL;
    size_t n = len / 2;
    uint8_t* buf = (uint8_t*)malloc(n);
    if (!buf) return NULL;
    for (size_t i = 0; i < n; i++) {
        unsigned int v;
        if (sscanf(hex + 2 * i, "%2x", &v) != 1) { free(buf); return NULL; }
        buf[i] = (uint8_t)v;
    }
    if (out_len) *out_len = n;
    return buf;
}

int main(void) {
    size_t k128_len, k192_len, k256_len;
    uint8_t* k128 = hexToBytes("00112233445566778899aabbccddeeff", &k128_len);
    uint8_t* k192 = hexToBytes("00112233445566778899aabbccddeeff0011223344556677", &k192_len);
    uint8_t* k256 = hexToBytes("000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f", &k256_len);

    const uint8_t data1[] = "secret-key-1";
    const uint8_t data2[] = "another-secret-key-2";
    const uint8_t data3[] = "K3";
    const uint8_t data4[] = {0x00,0x01,0x02,0x03,0xFF};
    const uint8_t data5[] = "A much longer piece of data to encrypt as key material.";

    char* c1 = encrypt_key(k128, k128_len, data1, sizeof(data1)-1);
    char* c2 = encrypt_key(k192, k192_len, data2, sizeof(data2)-1);
    char* c3 = encrypt_key(k256, k256_len, data3, sizeof(data3)-1);
    char* c4 = encrypt_key(k128, k128_len, data4, sizeof(data4));
    char* c5 = encrypt_key(k256, k256_len, data5, sizeof(data5)-1);

    printf("Test1: %s\n", c1 ? c1 : "(error)");
    printf("Test2: %s\n", c2 ? c2 : "(error)");
    printf("Test3: %s\n", c3 ? c3 : "(error)");
    printf("Test4: %s\n", c4 ? c4 : "(error)");
    printf("Test5: %s\n", c5 ? c5 : "(error)");

    free(k128); free(k192); free(k256);
    free(c1); free(c2); free(c3); free(c4); free(c5);

    return 0;
}