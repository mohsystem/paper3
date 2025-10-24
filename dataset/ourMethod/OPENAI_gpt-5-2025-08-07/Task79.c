#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

static char* b64encode(const unsigned char* data, size_t len) {
    static const char* tbl = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t out_len = ((len + 2) / 3) * 4;
    char* out = (char*)malloc(out_len + 1);
    if (!out) return NULL;
    size_t i = 0, j = 0;
    while (i + 3 <= len) {
        unsigned int n = (data[i] << 16) | (data[i+1] << 8) | data[i+2];
        out[j++] = tbl[(n >> 18) & 63];
        out[j++] = tbl[(n >> 12) & 63];
        out[j++] = tbl[(n >> 6) & 63];
        out[j++] = tbl[n & 63];
        i += 3;
    }
    if (i < len) {
        unsigned int n = data[i] << 16;
        if (i + 1 < len) n |= (data[i+1] << 8);
        out[j++] = tbl[(n >> 18) & 63];
        out[j++] = tbl[(n >> 12) & 63];
        if (i + 1 < len) {
            out[j++] = tbl[(n >> 6) & 63];
            out[j++] = '=';
        } else {
            out[j++] = '=';
            out[j++] = '=';
        }
    }
    out[j] = '\0';
    return out;
}

// Encrypts plaintext using passphrase-derived AES-256-GCM key and returns blob via out and out_len:
// [magic="ENC1"][version=1][salt(16)][iv(12)][ciphertext][tag(16)]
// Returns 0 on success, non-zero on failure.
static int encrypt(const char* passphrase, const unsigned char* plaintext, size_t plaintext_len,
                   unsigned char** out, size_t* out_len) {

    if (!passphrase || !*passphrase || !plaintext || !out || !out_len) return 1;
    if (plaintext_len > 50000000UL) return 2;

    unsigned char salt[16];
    unsigned char iv[12];
    if (RAND_bytes(salt, (int)sizeof(salt)) != 1) return 3;
    if (RAND_bytes(iv, (int)sizeof(iv)) != 1) return 4;

    unsigned char key[32];
    const int iterations = 210000;
    if (PKCS5_PBKDF2_HMAC(passphrase, (int)strlen(passphrase),
                          salt, (int)sizeof(salt),
                          iterations, EVP_sha256(),
                          (int)sizeof(key), key) != 1) {
        return 5;
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        OPENSSL_cleanse(key, sizeof(key));
        return 6;
    }

    int ret = 0;
    unsigned char* ciphertext = NULL;
    int outl = 0, tmplen = 0;
    unsigned char tag[16];

    do {
        if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) { ret = 7; break; }
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, (int)sizeof(iv), NULL) != 1) { ret = 8; break; }
        if (EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv) != 1) { ret = 9; break; }

        ciphertext = (unsigned char*)malloc(plaintext_len + 16);
        if (!ciphertext) { ret = 10; break; }

        if (EVP_EncryptUpdate(ctx, ciphertext, &outl, plaintext, (int)plaintext_len) != 1) { ret = 11; break; }
        if (EVP_EncryptFinal_ex(ctx, ciphertext + outl, &tmplen) != 1) { ret = 12; break; }
        outl += tmplen;

        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, (int)sizeof(tag), tag) != 1) { ret = 13; break; }

        size_t total = 4 + 1 + sizeof(salt) + sizeof(iv) + (size_t)outl + sizeof(tag);
        unsigned char* blob = (unsigned char*)malloc(total);
        if (!blob) { ret = 14; break; }

        size_t pos = 0;
        blob[pos++] = 'E'; blob[pos++] = 'N'; blob[pos++] = 'C'; blob[pos++] = '1';
        blob[pos++] = 1;
        memcpy(blob + pos, salt, sizeof(salt)); pos += sizeof(salt);
        memcpy(blob + pos, iv, sizeof(iv)); pos += sizeof(iv);
        memcpy(blob + pos, ciphertext, (size_t)outl); pos += (size_t)outl;
        memcpy(blob + pos, tag, sizeof(tag)); pos += sizeof(tag);

        *out = blob;
        *out_len = total;
    } while (0);

    OPENSSL_cleanse(key, sizeof(key));
    if (ciphertext) {
        OPENSSL_cleanse(ciphertext, (size_t)outl);
        free(ciphertext);
    }
    OPENSSL_cleanse(tag, sizeof(tag));
    EVP_CIPHER_CTX_free(ctx);

    return ret;
}

int main(void) {
    const char* pass_list[5] = {
        "correct horse battery staple",
        "Tr0ub4dor&3",
        "another strong passphrase",
        "Yet-Another-Secret",
        "SufficientlyLongAndComplexPassphrase!"
    };
    const char* msg_list[5] = {
        "Hello, world!",
        "Security-focused encryption using AES-256-GCM.",
        "短いメッセージ",
        "1234567890",
        "The quick brown fox jumps over the lazy dog."
    };

    for (int i = 0; i < 5; i++) {
        unsigned char* blob = NULL;
        size_t blob_len = 0;
        int rc = encrypt(pass_list[i], (const unsigned char*)msg_list[i], strlen(msg_list[i]), &blob, &blob_len);
        if (rc != 0) {
            printf("Encryption failed\n");
            continue;
        }
        char* b64 = b64encode(blob, blob_len);
        if (!b64) {
            printf("Encryption failed\n");
            free(blob);
            continue;
        }
        printf("%s\n", b64);
        OPENSSL_cleanse(blob, blob_len);
        free(b64);
        free(blob);
    }
    return 0;
}