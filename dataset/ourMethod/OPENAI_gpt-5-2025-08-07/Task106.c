#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/bn.h>
#include <openssl/crypto.h>
#include <openssl/err.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void die(const char* msg) {
    fprintf(stderr, "%s\n", msg);
    exit(1);
}

EVP_PKEY* generate_rsa_key(int bits) {
    if (bits < 2048) return NULL;
    BIGNUM* e = BN_new();
    if (!e) return NULL;
    if (BN_set_word(e, RSA_F4) != 1) { BN_free(e); return NULL; }
    RSA* rsa = RSA_new();
    if (!rsa) { BN_free(e); return NULL; }
    if (RSA_generate_key_ex(rsa, bits, e, NULL) != 1) {
        BN_free(e);
        RSA_free(rsa);
        return NULL;
    }
    BN_free(e);
    EVP_PKEY* pkey = EVP_PKEY_new();
    if (!pkey) { RSA_free(rsa); return NULL; }
    if (EVP_PKEY_assign_RSA(pkey, rsa) != 1) {
        EVP_PKEY_free(pkey);
        RSA_free(rsa);
        return NULL;
    }
    return pkey; /* pkey owns rsa */
}

int rsa_encrypt(EVP_PKEY* pub, const unsigned char* in, size_t inlen, unsigned char** out, size_t* outlen) {
    if (!pub || !in || !out || !outlen) return 0;
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pub, NULL);
    if (!ctx) return 0;
    if (EVP_PKEY_encrypt_init(ctx) != 1) { EVP_PKEY_CTX_free(ctx); return 0; }
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) != 1) { EVP_PKEY_CTX_free(ctx); return 0; }
    if (EVP_PKEY_CTX_set_rsa_oaep_md(ctx, EVP_sha256()) != 1) { EVP_PKEY_CTX_free(ctx); return 0; }
    if (EVP_PKEY_CTX_set_rsa_mgf1_md(ctx, EVP_sha256()) != 1) { EVP_PKEY_CTX_free(ctx); return 0; }
    size_t tmplen = 0;
    if (EVP_PKEY_encrypt(ctx, NULL, &tmplen, in, inlen) != 1) { EVP_PKEY_CTX_free(ctx); return 0; }
    unsigned char* buf = (unsigned char*)OPENSSL_malloc(tmplen);
    if (!buf) { EVP_PKEY_CTX_free(ctx); return 0; }
    if (EVP_PKEY_encrypt(ctx, buf, &tmplen, in, inlen) != 1) {
        OPENSSL_free(buf);
        EVP_PKEY_CTX_free(ctx);
        return 0;
    }
    *out = buf;
    *outlen = tmplen;
    EVP_PKEY_CTX_free(ctx);
    return 1;
}

int rsa_decrypt(EVP_PKEY* priv, const unsigned char* in, size_t inlen, unsigned char** out, size_t* outlen) {
    if (!priv || !in || !out || !outlen) return 0;
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(priv, NULL);
    if (!ctx) return 0;
    if (EVP_PKEY_decrypt_init(ctx) != 1) { EVP_PKEY_CTX_free(ctx); return 0; }
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) != 1) { EVP_PKEY_CTX_free(ctx); return 0; }
    if (EVP_PKEY_CTX_set_rsa_oaep_md(ctx, EVP_sha256()) != 1) { EVP_PKEY_CTX_free(ctx); return 0; }
    if (EVP_PKEY_CTX_set_rsa_mgf1_md(ctx, EVP_sha256()) != 1) { EVP_PKEY_CTX_free(ctx); return 0; }
    size_t tmplen = 0;
    if (EVP_PKEY_decrypt(ctx, NULL, &tmplen, in, inlen) != 1) { EVP_PKEY_CTX_free(ctx); return 0; }
    unsigned char* buf = (unsigned char*)OPENSSL_malloc(tmplen);
    if (!buf) { EVP_PKEY_CTX_free(ctx); return 0; }
    if (EVP_PKEY_decrypt(ctx, buf, &tmplen, in, inlen) != 1) {
        OPENSSL_free(buf);
        EVP_PKEY_CTX_free(ctx);
        return 0;
    }
    *out = buf;
    *outlen = tmplen;
    EVP_PKEY_CTX_free(ctx);
    return 1;
}

int main(void) {
    const char* messages[5] = {
        "Hello, RSA OAEP!",
        "The quick brown fox jumps over the lazy dog.",
        "RSA with OAEP SHA-256 and MGF1.",
        "Data 1234567890!@#$%^&*()",
        "Unicode: こんにちは世界🌐"
    };

    EVP_PKEY* pkey = generate_rsa_key(2048);
    if (!pkey) die("Key generation failed");

    size_t passed = 0;
    for (int i = 0; i < 5; i++) {
        const unsigned char* pt = (const unsigned char*)messages[i];
        size_t ptlen = strlen(messages[i]);
        unsigned char* ct = NULL; size_t ctlen = 0;
        unsigned char* dec = NULL; size_t declen = 0;

        int ok = rsa_encrypt(pkey, pt, ptlen, &ct, &ctlen);
        if (!ok) {
            printf("Test %d: FAIL (encrypt)\n", i + 1);
            continue;
        }
        ok = rsa_decrypt(pkey, ct, ctlen, &dec, &declen);
        if (!ok) {
            OPENSSL_free(ct);
            printf("Test %d: FAIL (decrypt)\n", i + 1);
            continue;
        }
        int eq = (ptlen == declen) && (CRYPTO_memcmp(pt, dec, ptlen) == 0);
        if (eq) passed++;
        printf("Test %d: %s | Ciphertext length=%zu\n", i + 1, eq ? "OK" : "FAIL", ctlen);

        OPENSSL_free(ct);
        OPENSSL_free(dec);
    }
    printf("Passed %zu of 5 tests.\n", passed);
    EVP_PKEY_free(pkey);
    return 0;
}