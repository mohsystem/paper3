#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/crypto.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    SALT_LEN = 16,
    IV_LEN   = 12,
    KEY_LEN  = 32,
    TAG_LEN  = 16,
    PBKDF2_ITERS = 210000
};

static const unsigned char MAGIC[4] = { 'E','N','C','1' };
static const unsigned char VERSION = 1;

typedef struct {
    unsigned char* data;
    size_t len;
} ByteArray;

static void bytearray_free(ByteArray* ba) {
    if (!ba) return;
    if (ba->data) {
        OPENSSL_cleanse(ba->data, ba->len);
        free(ba->data);
    }
    ba->data = NULL;
    ba->len = 0;
}

static int derive_key(const char* passphrase, const unsigned char* salt, size_t salt_len,
                      unsigned char* out_key, size_t out_key_len) {
    if (!passphrase || !salt || salt_len != SALT_LEN || !out_key || out_key_len != KEY_LEN) return 0;
    int ok = PKCS5_PBKDF2_HMAC(passphrase, (int)strlen(passphrase),
                               salt, (int)salt_len,
                               PBKDF2_ITERS,
                               EVP_sha256(),
                               (int)out_key_len,
                               out_key);
    return ok == 1;
}

// AES-256-GCM encryption with PBKDF2-HMAC-SHA256.
// Output layout: [magic(4)][version(1)][salt(16)][iv(12)][ciphertext(N)][tag(16)]
static ByteArray encrypt_data(const char* passphrase, const unsigned char* plaintext, size_t plaintext_len) {
    ByteArray out = { NULL, 0 };
    unsigned char salt[SALT_LEN];
    unsigned char iv[IV_LEN];
    unsigned char key[KEY_LEN];

    if (!passphrase) return out;

    if (RAND_bytes(salt, SALT_LEN) != 1) {
        return out;
    }
    if (RAND_bytes(iv, IV_LEN) != 1) {
        return out;
    }
    if (!derive_key(passphrase, salt, SALT_LEN, key, KEY_LEN)) {
        OPENSSL_cleanse(key, KEY_LEN);
        return out;
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        OPENSSL_cleanse(key, KEY_LEN);
        return out;
    }

    int ok = 1;
    ok &= EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) == 1;
    ok &= EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, IV_LEN, NULL) == 1;
    ok &= EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv) == 1;

    unsigned char* ciphertext = NULL;
    int out_len = 0;
    int total_len = 0;
    if (ok) {
        ciphertext = (unsigned char*)malloc(plaintext_len > 0 ? plaintext_len : 1);
        if (!ciphertext) {
            ok = 0;
        }
    }

    if (ok && plaintext_len > 0) {
        ok &= EVP_EncryptUpdate(ctx, ciphertext, &out_len, plaintext, (int)plaintext_len) == 1;
        total_len += out_len;
    }

    if (ok) {
        ok &= EVP_EncryptFinal_ex(ctx, NULL, &out_len) == 1;
    }

    unsigned char tag[TAG_LEN];
    if (ok) {
        ok &= EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_LEN, tag) == 1;
    }

    if (ok) {
        size_t total_size = sizeof(MAGIC) + 1 + SALT_LEN + IV_LEN + (size_t)total_len + TAG_LEN;
        unsigned char* buffer = (unsigned char*)malloc(total_size);
        if (buffer) {
            size_t off = 0;
            memcpy(buffer + off, MAGIC, sizeof(MAGIC)); off += sizeof(MAGIC);
            buffer[off++] = VERSION;
            memcpy(buffer + off, salt, SALT_LEN); off += SALT_LEN;
            memcpy(buffer + off, iv, IV_LEN); off += IV_LEN;
            if (total_len > 0) {
                memcpy(buffer + off, ciphertext, (size_t)total_len); off += (size_t)total_len;
            }
            memcpy(buffer + off, tag, TAG_LEN); off += TAG_LEN;

            out.data = buffer;
            out.len = total_size;
        }
    }

    if (ciphertext) {
        OPENSSL_cleanse(ciphertext, (size_t)total_len);
        free(ciphertext);
    }
    EVP_CIPHER_CTX_free(ctx);
    OPENSSL_cleanse(key, KEY_LEN);
    return out;
}

// Compute SHA-256 of data and return hex string. Caller must free.
static char* sha256_hex(const unsigned char* data, size_t len) {
    unsigned char md[SHA256_DIGEST_LENGTH];
    SHA256_CTX c;
    if (SHA256_Init(&c) != 1) return NULL;
    if (len > 0 && data) {
        if (SHA256_Update(&c, data, len) != 1) return NULL;
    }
    if (SHA256_Final(md, &c) != 1) return NULL;

    char* hex = (char*)malloc(SHA256_DIGEST_LENGTH * 2 + 1);
    if (!hex) return NULL;
    for (size_t i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        static const char* hexd = "0123456789abcdef";
        hex[2*i]   = hexd[(md[i] >> 4) & 0xF];
        hex[2*i+1] = hexd[md[i] & 0xF];
    }
    hex[SHA256_DIGEST_LENGTH * 2] = '\0';
    OPENSSL_cleanse(md, sizeof(md));
    return hex;
}

int main(void) {
    const char* envPass = getenv("TASK72_PASSPHRASE");
    const char* passphrase = envPass ? envPass : "passphrase-for-tests-only";

    const unsigned char msg1[] = ""; // empty
    const unsigned char msg2[] = "Hello, world!";
    const unsigned char msg3[] = "The quick brown fox jumps over the lazy dog";
    const unsigned char msg4[] = "Data encryption test vector #4 with different length...";
    const unsigned char msg5[] = "Unicode: \xF0\x9F\x94\x92 \xE2\x9C\x94";

    const unsigned char* msgs[5] = { msg1, msg2, msg3, msg4, msg5 };
    size_t lens[5] = {
        0,
        sizeof(msg2) - 1,
        sizeof(msg3) - 1,
        sizeof(msg4) - 1,
        sizeof(msg5) - 1
    };

    for (int i = 0; i < 5; ++i) {
        ByteArray enc = encrypt_data(passphrase, msgs[i], lens[i]);
        char* hex = sha256_hex(enc.data, enc.len);
        if (hex) {
            printf("Test %d: output_len=%zu, sha256=%s\n", i+1, enc.len, hex);
            free(hex);
        } else {
            printf("Test %d: output_len=%zu, sha256=<error>\n", i+1, enc.len);
        }
        bytearray_free(&enc);
    }
    return 0;
}