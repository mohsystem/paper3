#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

// File format: [magic="ENC1"(4)][version=1(1)][salt(16)][iv(12)][ciphertext(...)] [tag(16)]
static const unsigned char MAGIC[4] = {'E','N','C','1'};
static const unsigned char VERSION = 1;
enum { SALT_LEN = 16, IV_LEN = 12, TAG_LEN = 16, KEY_LEN = 32, PBKDF2_ITERS = 210000 };

typedef struct {
    uint8_t* data;
    size_t len;
} Buffer;

static void secure_clean(void* ptr, size_t len) {
#if defined(OPENSSL_cleanse)
    OPENSSL_cleanse(ptr, len);
#else
    volatile unsigned char* p = (volatile unsigned char*)ptr;
    while (len--) *p++ = 0;
#endif
}

static bool derive_key(const char* pass, const unsigned char* salt, unsigned char* out_key, size_t out_len) {
    if (pass == NULL) return false;
    size_t pass_len = strnlen(pass, 1025);
    if (pass_len == 1025) return false; // too long
    int ok = PKCS5_PBKDF2_HMAC(pass, (int)pass_len, salt, SALT_LEN, PBKDF2_ITERS, EVP_sha256(), (int)out_len, out_key);
    return ok == 1;
}

static Buffer make_empty(void) {
    Buffer b; b.data = NULL; b.len = 0; return b;
}

static Buffer encrypt_message(const char* passphrase, const uint8_t* plaintext, size_t pt_len) {
    Buffer out = make_empty();
    if (!passphrase || !plaintext || pt_len > ((size_t)1 << 30) || strlen(passphrase) == 0) {
        return out;
    }

    unsigned char salt[SALT_LEN];
    unsigned char iv[IV_LEN];
    unsigned char key[KEY_LEN];
    memset(key, 0, sizeof(key));

    if (RAND_bytes(salt, SALT_LEN) != 1 || RAND_bytes(iv, IV_LEN) != 1) {
        secure_clean(key, sizeof(key));
        return out;
    }
    if (!derive_key(passphrase, salt, key, sizeof(key))) {
        secure_clean(key, sizeof(key));
        return out;
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) { secure_clean(key, sizeof(key)); return out; }

    int ok = EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL);
    if (ok != 1) { EVP_CIPHER_CTX_free(ctx); secure_clean(key, sizeof(key)); return out; }

    ok = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, IV_LEN, NULL);
    if (ok != 1) { EVP_CIPHER_CTX_free(ctx); secure_clean(key, sizeof(key)); return out; }

    ok = EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv);
    if (ok != 1) { EVP_CIPHER_CTX_free(ctx); secure_clean(key, sizeof(key)); return out; }

    uint8_t* ciphertext = NULL;
    if (pt_len > 0) {
        ciphertext = (uint8_t*)malloc(pt_len);
        if (!ciphertext) { EVP_CIPHER_CTX_free(ctx); secure_clean(key, sizeof(key)); return out; }
    }

    int outlen = 0, total = 0;
    if (pt_len > 0) {
        ok = EVP_EncryptUpdate(ctx, ciphertext, &outlen, plaintext, (int)pt_len);
        if (ok != 1) { free(ciphertext); EVP_CIPHER_CTX_free(ctx); secure_clean(key, sizeof(key)); return out; }
        total = outlen;
    }

    ok = EVP_EncryptFinal_ex(ctx, (pt_len > 0) ? (ciphertext + total) : NULL, &outlen);
    if (ok != 1) { if (ciphertext) free(ciphertext); EVP_CIPHER_CTX_free(ctx); secure_clean(key, sizeof(key)); return out; }
    total += outlen;

    unsigned char tag[TAG_LEN];
    ok = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_LEN, tag);
    if (ok != 1) { if (ciphertext) free(ciphertext); EVP_CIPHER_CTX_free(ctx); secure_clean(key, sizeof(key)); return out; }

    // Build blob
    size_t blob_len = 4 + 1 + SALT_LEN + IV_LEN + (size_t)total + TAG_LEN;
    uint8_t* blob = (uint8_t*)malloc(blob_len);
    if (!blob) {
        if (ciphertext) free(ciphertext);
        EVP_CIPHER_CTX_free(ctx);
        secure_clean(key, sizeof(key));
        return out;
    }

    size_t off = 0;
    memcpy(blob + off, MAGIC, 4); off += 4;
    blob[off++] = VERSION;
    memcpy(blob + off, salt, SALT_LEN); off += SALT_LEN;
    memcpy(blob + off, iv, IV_LEN); off += IV_LEN;
    if (total > 0) memcpy(blob + off, ciphertext, (size_t)total);
    off += (size_t)total;
    memcpy(blob + off, tag, TAG_LEN); off += TAG_LEN;

    out.data = blob;
    out.len = blob_len;

    if (ciphertext) free(ciphertext);
    EVP_CIPHER_CTX_free(ctx);
    secure_clean(key, sizeof(key));
    return out;
}

static Buffer decrypt_message(const char* passphrase, const uint8_t* blob, size_t blob_len) {
    Buffer out = make_empty();
    if (!passphrase || !blob || blob_len < 4 + 1 + SALT_LEN + IV_LEN + TAG_LEN || strlen(passphrase) == 0) {
        return out;
    }
    if (!(blob[0]==MAGIC[0] && blob[1]==MAGIC[1] && blob[2]==MAGIC[2] && blob[3]==MAGIC[3])) {
        return out;
    }
    if (blob[4] != VERSION) {
        return out;
    }
    size_t off = 5;
    const unsigned char* salt = blob + off; off += SALT_LEN;
    const unsigned char* iv = blob + off; off += IV_LEN;
    if (blob_len < off + TAG_LEN) return out;
    size_t ct_len = blob_len - off - TAG_LEN;
    const unsigned char* ciphertext = blob + off;
    const unsigned char* tag = blob + off + ct_len;

    unsigned char key[KEY_LEN];
    memset(key, 0, sizeof(key));
    if (!derive_key(passphrase, salt, key, sizeof(key))) {
        secure_clean(key, sizeof(key));
        return out;
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) { secure_clean(key, sizeof(key)); return out; }

    int ok = EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL);
    if (ok != 1) { EVP_CIPHER_CTX_free(ctx); secure_clean(key, sizeof(key)); return out; }

    ok = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, IV_LEN, NULL);
    if (ok != 1) { EVP_CIPHER_CTX_free(ctx); secure_clean(key, sizeof(key)); return out; }

    ok = EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv);
    if (ok != 1) { EVP_CIPHER_CTX_free(ctx); secure_clean(key, sizeof(key)); return out; }

    uint8_t* plaintext = NULL;
    if (ct_len > 0) {
        plaintext = (uint8_t*)malloc(ct_len);
        if (!plaintext) { EVP_CIPHER_CTX_free(ctx); secure_clean(key, sizeof(key)); return out; }
    }

    int outlen = 0, total = 0;
    if (ct_len > 0) {
        ok = EVP_DecryptUpdate(ctx, plaintext, &outlen, ciphertext, (int)ct_len);
        if (ok != 1) { if (plaintext) free(plaintext); EVP_CIPHER_CTX_free(ctx); secure_clean(key, sizeof(key)); return make_empty(); }
        total = outlen;
    }

    ok = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, TAG_LEN, (void*)tag);
    if (ok != 1) { if (plaintext) free(plaintext); EVP_CIPHER_CTX_free(ctx); secure_clean(key, sizeof(key)); return make_empty(); }

    ok = EVP_DecryptFinal_ex(ctx, (ct_len > 0) ? (plaintext + total) : NULL, &outlen);
    if (ok != 1) { // auth failed
        if (plaintext) { secure_clean(plaintext, ct_len); free(plaintext); }
        EVP_CIPHER_CTX_free(ctx);
        secure_clean(key, sizeof(key));
        return make_empty();
    }
    total += outlen;

    out.data = plaintext;
    out.len = (size_t)total;

    EVP_CIPHER_CTX_free(ctx);
    secure_clean(key, sizeof(key));
    return out;
}

static void free_buffer(Buffer* b) {
    if (!b) return;
    if (b->data) {
        secure_clean(b->data, b->len);
        free(b->data);
        b->data = NULL;
        b->len = 0;
    }
}

static void print_test_result(int idx, bool success, size_t enc_size) {
    printf("Test %d: %s | enc_size=%zu\n", idx, success ? "OK" : "FAIL", enc_size);
}

int main(void) {
    // Test cases
    const char* pass1 = "correct horse battery staple";
    const char* msg1 = "Hello, world!";

    const char* pass2 = "p@ssw0rd! with unicode ñ";
    const char* msg2 = "The quick brown fox jumps over the lazy dog 12345.";

    const char* pass3 = "another strong passphrase";
    size_t len3 = 1000;
    char* msg3 = (char*)malloc(len3 + 1);
    if (!msg3) return 1;
    memset(msg3, 'A', len3);
    msg3[len3] = '\0';

    const char* pass4 = "another pass";
    const char* msg4 = "Short";

    const char* pass5 = "🧪 emojis";
    const char* msg5 = "This message contains emojis 😀🚀 and symbols ©®";

    struct Case { const char* pass; const uint8_t* msg; size_t msg_len; } cases[5];
    cases[0].pass = pass1; cases[0].msg = (const uint8_t*)msg1; cases[0].msg_len = strlen(msg1);
    cases[1].pass = pass2; cases[1].msg = (const uint8_t*)msg2; cases[1].msg_len = strlen(msg2);
    cases[2].pass = pass3; cases[2].msg = (const uint8_t*)msg3; cases[2].msg_len = strlen(msg3);
    cases[3].pass = pass4; cases[3].msg = (const uint8_t*)msg4; cases[3].msg_len = strlen(msg4);
    cases[4].pass = pass5; cases[4].msg = (const uint8_t*)msg5; cases[4].msg_len = strlen(msg5);

    for (int i = 0; i < 5; ++i) {
        Buffer enc = encrypt_message(cases[i].pass, cases[i].msg, cases[i].msg_len);
        bool success = false;
        if (enc.data != NULL && enc.len > 0) {
            Buffer dec = decrypt_message(cases[i].pass, enc.data, enc.len);
            if (dec.data != NULL && dec.len == cases[i].msg_len) {
                success = (memcmp(dec.data, cases[i].msg, cases[i].msg_len) == 0);
            }
            free_buffer(&dec);
        }
        print_test_result(i+1, success, enc.len);
        free_buffer(&enc);
    }

    free(msg3);
    return 0;
}