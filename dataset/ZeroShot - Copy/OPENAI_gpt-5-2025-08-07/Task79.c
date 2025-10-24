#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/crypto.h>

static char* b64encode(const unsigned char* data, size_t len) {
    size_t out_len = 4 * ((len + 2) / 3);
    char* out = (char*)malloc(out_len + 1);
    if (!out) return NULL;
    int written = EVP_EncodeBlock((unsigned char*)out, data, (int)len);
    if (written < 0) {
        free(out);
        return NULL;
    }
    if ((size_t)written < out_len) {
        // pad with '=' already handled by EVP_EncodeBlock; adjust size
        out[written] = '\0';
        return out;
    }
    out[out_len] = '\0';
    return out;
}

char* encrypt(const char* message, const char* secret) {
    if (!message) message = "";
    if (!secret) secret = "";

    const int saltLen = 16;
    const int ivLen = 12;
    const int keyLen = 32;
    const int iterations = 200000;

    unsigned char salt[saltLen];
    unsigned char iv[ivLen];
    unsigned char key[keyLen];

    if (RAND_bytes(salt, saltLen) != 1) return NULL;
    if (RAND_bytes(iv, ivLen) != 1) return NULL;

    if (PKCS5_PBKDF2_HMAC(secret, (int)strlen(secret), salt, saltLen,
                          iterations, EVP_sha256(), keyLen, key) != 1) {
        OPENSSL_cleanse(key, keyLen);
        return NULL;
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        OPENSSL_cleanse(key, keyLen);
        return NULL;
    }

    int outlen = 0, tmplen = 0;
    size_t msg_len = strlen(message);
    unsigned char* ciphertext = (unsigned char*)malloc(msg_len + 16); // ciphertext only (tag appended later)
    if (!ciphertext) {
        EVP_CIPHER_CTX_free(ctx);
        OPENSSL_cleanse(key, keyLen);
        return NULL;
    }

    int ok = 1;
    do {
        if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) { ok = 0; break; }
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, ivLen, NULL) != 1) { ok = 0; break; }
        if (EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv) != 1) { ok = 0; break; }

        if (EVP_EncryptUpdate(ctx, ciphertext, &outlen, (const unsigned char*)message, (int)msg_len) != 1) { ok = 0; break; }

        if (EVP_EncryptFinal_ex(ctx, ciphertext + outlen, &tmplen) != 1) { ok = 0; break; }
        int ct_len = outlen + tmplen;

        unsigned char tag[16];
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag) != 1) { ok = 0; break; }

        // Build combined ciphertext + tag
        size_t combined_len = (size_t)ct_len + 16;
        unsigned char* combined = (unsigned char*)malloc(combined_len);
        if (!combined) { ok = 0; break; }
        memcpy(combined, ciphertext, (size_t)ct_len);
        memcpy(combined + ct_len, tag, 16);

        char* sSalt = b64encode(salt, saltLen);
        char* sIv = b64encode(iv, ivLen);
        char* sCt = b64encode(combined, combined_len);
        free(combined);

        if (!sSalt || !sIv || !sCt) {
            free(sSalt); free(sIv); free(sCt);
            ok = 0; break;
        }

        size_t final_len = strlen(sSalt) + 1 + strlen(sIv) + 1 + strlen(sCt) + 1;
        char* result = (char*)malloc(final_len);
        if (!result) {
            free(sSalt); free(sIv); free(sCt);
            ok = 0; break;
        }
        snprintf(result, final_len, "%s:%s:%s", sSalt, sIv, sCt);
        free(sSalt); free(sIv); free(sCt);

        EVP_CIPHER_CTX_free(ctx);
        OPENSSL_cleanse(key, keyLen);
        free(ciphertext);
        return result;
    } while (0);

    EVP_CIPHER_CTX_free(ctx);
    OPENSSL_cleanse(key, keyLen);
    free(ciphertext);
    return NULL;
}

int main(void) {
    char* r1 = encrypt("Hello, World!", "S3cr3t!Key");
    char* r2 = encrypt("Attack at dawn", "another$trongKey123");
    char* r3 = encrypt("1234567890", "password");
    char* r4 = encrypt("Unicode ✓ © ☕", "𝒦𝑒𝓎🔒");
    char* r5 = encrypt("", "empty-message-key");

    if (r1) { printf("%s\n", r1); free(r1); } else { printf("(null)\n"); }
    if (r2) { printf("%s\n", r2); free(r2); } else { printf("(null)\n"); }
    if (r3) { printf("%s\n", r3); free(r3); } else { printf("(null)\n"); }
    if (r4) { printf("%s\n", r4); free(r4); } else { printf("(null)\n"); }
    if (r5) { printf("%s\n", r5); free(r5); } else { printf("(null)\n"); }

    return 0;
}