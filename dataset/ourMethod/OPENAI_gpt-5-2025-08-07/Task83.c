/*
Secure AES-256-CBC Encrypt-then-MAC (EtM) for encrypting a provided key.

Build (example):
  gcc -Wall -Wextra -O2 task83.c -lcrypto -o task83

This program:
- Derives encryption and MAC keys from a passphrase using PBKDF2-HMAC-SHA-256 (210,000 iterations) and a random 16-byte salt.
- Generates a random 16-byte IV for AES-CBC.
- Encrypts the provided key with AES-256-CBC (PKCS#7 padding).
- Computes HMAC-SHA-256 over [magic|version|salt|iv|ciphertext] for integrity (Encrypt-then-MAC).
- Produces a binary package: [magic="ENC1"(4)][version=1(1)][salt(16)][iv(16)][ciphertext(...)] [tag(32)]

It includes 5 tests that do not print sensitive materials:
  1) Roundtrip with 32-byte key.
  2) Roundtrip with 16-byte key.
  3) Roundtrip with 48-byte key.
  4) Decrypt with wrong passphrase (expected failure).
  5) Tamper ciphertext to ensure MAC verification fails (expected failure).
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/hmac.h>
#include <openssl/crypto.h>

#define MAGIC "ENC1"
#define MAGIC_LEN 4
#define VERSION 1u
#define VERSION_LEN 1

#define SALT_LEN 16u
#define IV_LEN 16u
#define ENC_KEY_LEN 32u
#define MAC_KEY_LEN 32u
#define TAG_LEN 32u
#define PBKDF2_ITERS 210000u

#define AES_BLOCK_SIZE 16u

/* ================= Utility: secure memory zero ================= */
static void secure_memzero(void *ptr, size_t len) {
#if defined(OPENSSL_cleanse)
    OPENSSL_cleanse(ptr, len);
#else
    volatile unsigned char *p = (volatile unsigned char *)ptr;
    while (len--) {
        *p++ = 0;
    }
#endif
}

/* ================= Utility: safe size_t addition ================= */
static int safe_size_add(size_t a, size_t b, size_t *out) {
    if (SIZE_MAX - a < b) return 0;
    *out = a + b;
    return 1;
}

/* ================= Constant-time comparison ================= */
static int consttime_eq(const unsigned char *a, const unsigned char *b, size_t len) {
#if defined(HAVE_CRYPTO_MEMCMP) || (OPENSSL_VERSION_NUMBER >= 0x10100000L)
    return CRYPTO_memcmp(a, b, len) == 0 ? 1 : 0;
#else
    unsigned char diff = 0;
    for (size_t i = 0; i < len; i++) diff |= (unsigned char)(a[i] ^ b[i]);
    return diff == 0 ? 1 : 0;
#endif
}

/* ================= Key derivation (PBKDF2-HMAC-SHA256) ================= */
static int derive_keys_pbkdf2(const char *pass, size_t pass_len,
                              const unsigned char *salt, size_t salt_len,
                              unsigned char *out_enc_key, unsigned char *out_mac_key) {
    if (!pass || !salt || !out_enc_key || !out_mac_key) return 0;

    unsigned char keymat[ENC_KEY_LEN + MAC_KEY_LEN];
    if (PKCS5_PBKDF2_HMAC(pass, (int)pass_len, salt, (int)salt_len,
                          (int)PBKDF2_ITERS, EVP_sha256(),
                          (int)(sizeof(keymat)), keymat) != 1) {
        return 0;
    }
    memcpy(out_enc_key, keymat, ENC_KEY_LEN);
    memcpy(out_mac_key, keymat + ENC_KEY_LEN, MAC_KEY_LEN);
    secure_memzero(keymat, sizeof(keymat));
    return 1;
}

/* ================= AES-256-CBC Encrypt ================= */
static int aes256_cbc_encrypt(const unsigned char *plaintext, size_t plaintext_len,
                              const unsigned char *key, const unsigned char *iv,
                              unsigned char **ciphertext, size_t *ciphertext_len) {
    if (!plaintext || !key || !iv || !ciphertext || !ciphertext_len) return 0;

    int ok = 0;
    EVP_CIPHER_CTX *ctx = NULL;
    unsigned char *out = NULL;
    int outlen1 = 0, outlen2 = 0;

    ctx = EVP_CIPHER_CTX_new();
    if (!ctx) goto cleanup;

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1) goto cleanup;

    size_t alloc_len = 0;
    if (!safe_size_add(plaintext_len, AES_BLOCK_SIZE, &alloc_len)) goto cleanup;
    out = (unsigned char *)malloc(alloc_len);
    if (!out) goto cleanup;

    if (EVP_EncryptUpdate(ctx, out, &outlen1, plaintext, (int)plaintext_len) != 1) goto cleanup;
    if (EVP_EncryptFinal_ex(ctx, out + outlen1, &outlen2) != 1) goto cleanup;

    *ciphertext_len = (size_t)outlen1 + (size_t)outlen2;
    *ciphertext = out; out = NULL;
    ok = 1;

cleanup:
    if (ctx) EVP_CIPHER_CTX_free(ctx);
    if (out) {
        secure_memzero(out, alloc_len);
        free(out);
    }
    return ok;
}

/* ================= AES-256-CBC Decrypt ================= */
static int aes256_cbc_decrypt(const unsigned char *ciphertext, size_t ciphertext_len,
                              const unsigned char *key, const unsigned char *iv,
                              unsigned char **plaintext, size_t *plaintext_len) {
    if (!ciphertext || !key || !iv || !plaintext || !plaintext_len) return 0;

    int ok = 0;
    EVP_CIPHER_CTX *ctx = NULL;
    unsigned char *out = NULL;
    int outlen1 = 0, outlen2 = 0;

    ctx = EVP_CIPHER_CTX_new();
    if (!ctx) goto cleanup;

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1) goto cleanup;

    out = (unsigned char *)malloc(ciphertext_len);
    if (!out) goto cleanup;

    if (EVP_DecryptUpdate(ctx, out, &outlen1, ciphertext, (int)ciphertext_len) != 1) goto cleanup;

    if (EVP_DecryptFinal_ex(ctx, out + outlen1, &outlen2) != 1) goto cleanup;

    *plaintext_len = (size_t)outlen1 + (size_t)outlen2;
    *plaintext = out; out = NULL;
    ok = 1;

cleanup:
    if (ctx) EVP_CIPHER_CTX_free(ctx);
    if (out) {
        secure_memzero(out, ciphertext_len);
        free(out);
    }
    return ok;
}

/* ================= HMAC-SHA-256 ================= */
static int compute_hmac_sha256(const unsigned char *data, size_t data_len,
                               const unsigned char *mac_key, size_t mac_key_len,
                               unsigned char out_tag[TAG_LEN]) {
    if (!data || !mac_key || !out_tag) return 0;

    int ok = 0;
    HMAC_CTX *hctx = HMAC_CTX_new();
    if (!hctx) return 0;

    if (HMAC_Init_ex(hctx, mac_key, (int)mac_key_len, EVP_sha256(), NULL) != 1) goto cleanup;
    if (HMAC_Update(hctx, data, data_len) != 1) goto cleanup;

    unsigned int tag_len = 0;
    if (HMAC_Final(hctx, out_tag, &tag_len) != 1) goto cleanup;
    if (tag_len != TAG_LEN) goto cleanup;

    ok = 1;

cleanup:
    HMAC_CTX_free(hctx);
    return ok;
}

/* ================= Package Format Helpers ================= */
/*
Package: [magic(4)="ENC1"][version(1)=1][salt(16)][iv(16)][ciphertext(var)][tag(32)]
*/
static int build_package(const unsigned char salt[SALT_LEN],
                         const unsigned char iv[IV_LEN],
                         const unsigned char *ciphertext, size_t ciphertext_len,
                         const unsigned char tag[TAG_LEN],
                         unsigned char **out_pkg, size_t *out_pkg_len) {
    if (!salt || !iv || !ciphertext || ciphertext_len == 0 || !tag || !out_pkg || !out_pkg_len) return 0;

    size_t header_len = MAGIC_LEN + VERSION_LEN + SALT_LEN + IV_LEN;
    size_t tmp = 0, total = 0;

    if (!safe_size_add(header_len, ciphertext_len, &tmp)) return 0;
    if (!safe_size_add(tmp, TAG_LEN, &total)) return 0;

    unsigned char *pkg = (unsigned char *)malloc(total);
    if (!pkg) return 0;

    size_t off = 0;
    memcpy(pkg + off, MAGIC, MAGIC_LEN); off += MAGIC_LEN;
    pkg[off++] = (unsigned char)VERSION;
    memcpy(pkg + off, salt, SALT_LEN); off += SALT_LEN;
    memcpy(pkg + off, iv, IV_LEN); off += IV_LEN;
    memcpy(pkg + off, ciphertext, ciphertext_len); off += ciphertext_len;
    memcpy(pkg + off, tag, TAG_LEN); off += TAG_LEN;

    *out_pkg = pkg;
    *out_pkg_len = total;
    return 1;
}

static int parse_package(const unsigned char *pkg, size_t pkg_len,
                         const unsigned char **salt, const unsigned char **iv,
                         const unsigned char **ciphertext, size_t *ciphertext_len,
                         const unsigned char **tag) {
    if (!pkg || pkg_len < MAGIC_LEN + VERSION_LEN + SALT_LEN + IV_LEN + TAG_LEN) return 0;

    size_t off = 0;

    if (memcmp(pkg + off, MAGIC, MAGIC_LEN) != 0) return 0; off += MAGIC_LEN;

    unsigned char version = pkg[off++]; if (version != (unsigned char)VERSION) return 0;

    *salt = pkg + off; off += SALT_LEN;
    *iv = pkg + off; off += IV_LEN;

    if (pkg_len < off + TAG_LEN) return 0;
    size_t ct_len = pkg_len - off - TAG_LEN;
    if (ct_len == 0) return 0;

    *ciphertext = pkg + off; *ciphertext_len = ct_len; off += ct_len;
    *tag = pkg + off;

    return 1;
}

/* ================= Public API: Encrypt given key ================= */
int encrypt_key_with_aes_cbc(const unsigned char *key_to_encrypt, size_t key_len,
                             const char *passphrase, size_t pass_len,
                             unsigned char **out_encrypted_pkg, size_t *out_encrypted_pkg_len) {
    if (!key_to_encrypt || key_len == 0 || !passphrase || pass_len == 0 || !out_encrypted_pkg || !out_encrypted_pkg_len) {
        return 0;
    }

    int ok = 0;
    unsigned char salt[SALT_LEN];
    unsigned char iv[IV_LEN];
    unsigned char enc_key[ENC_KEY_LEN];
    unsigned char mac_key[MAC_KEY_LEN];
    unsigned char *ciphertext = NULL;
    size_t ciphertext_len = 0;
    unsigned char tag[TAG_LEN];

    if (RAND_bytes(salt, (int)SALT_LEN) != 1) goto cleanup;
    if (RAND_bytes(iv, (int)IV_LEN) != 1) goto cleanup;

    if (!derive_keys_pbkdf2(passphrase, pass_len, salt, SALT_LEN, enc_key, mac_key)) goto cleanup;

    if (!aes256_cbc_encrypt(key_to_encrypt, key_len, enc_key, iv, &ciphertext, &ciphertext_len)) goto cleanup;

    /* HMAC over header + ciphertext */
    size_t header_len = MAGIC_LEN + VERSION_LEN + SALT_LEN + IV_LEN;
    size_t hmac_input_len = 0, tmp = 0;
    if (!safe_size_add(header_len, ciphertext_len, &hmac_input_len)) goto cleanup;

    unsigned char *hmac_input = (unsigned char *)malloc(hmac_input_len);
    if (!hmac_input) goto cleanup;

    size_t off = 0;
    memcpy(hmac_input + off, MAGIC, MAGIC_LEN); off += MAGIC_LEN;
    hmac_input[off++] = (unsigned char)VERSION;
    memcpy(hmac_input + off, salt, SALT_LEN); off += SALT_LEN;
    memcpy(hmac_input + off, iv, IV_LEN); off += IV_LEN;
    memcpy(hmac_input + off, ciphertext, ciphertext_len); off += ciphertext_len;

    if (!compute_hmac_sha256(hmac_input, hmac_input_len, mac_key, MAC_KEY_LEN, tag)) {
        secure_memzero(hmac_input, hmac_input_len);
        free(hmac_input);
        goto cleanup;
    }

    secure_memzero(hmac_input, hmac_input_len);
    free(hmac_input);

    if (!build_package(salt, iv, ciphertext, ciphertext_len, tag, out_encrypted_pkg, out_encrypted_pkg_len)) goto cleanup;

    ok = 1;

cleanup:
    if (ciphertext) {
        secure_memzero(ciphertext, ciphertext_len);
        free(ciphertext);
    }
    secure_memzero(enc_key, sizeof(enc_key));
    secure_memzero(mac_key, sizeof(mac_key));
    secure_memzero(iv, sizeof(iv));
    secure_memzero(salt, sizeof(salt));
    secure_memzero(tag, sizeof(tag));
    return ok;
}

/* ================= Optional: Decrypt for validation/testing ================= */
int decrypt_key_with_aes_cbc(const unsigned char *pkg, size_t pkg_len,
                             const char *passphrase, size_t pass_len,
                             unsigned char **out_plain, size_t *out_plain_len) {
    if (!pkg || pkg_len == 0 || !passphrase || pass_len == 0 || !out_plain || !out_plain_len) return 0;

    int ok = 0;
    const unsigned char *salt = NULL, *iv = NULL, *ciphertext = NULL, *tag = NULL;
    size_t ciphertext_len = 0;

    if (!parse_package(pkg, pkg_len, &salt, &iv, &ciphertext, &ciphertext_len, &tag)) return 0;

    unsigned char enc_key[ENC_KEY_LEN];
    unsigned char mac_key[MAC_KEY_LEN];

    if (!derive_keys_pbkdf2(passphrase, pass_len, salt, SALT_LEN, enc_key, mac_key)) goto cleanup;

    /* Recompute HMAC and verify in constant time */
    size_t header_len = MAGIC_LEN + VERSION_LEN + SALT_LEN + IV_LEN;
    size_t hmac_input_len = 0;
    if (!safe_size_add(header_len, ciphertext_len, &hmac_input_len)) goto cleanup;

    unsigned char *hmac_input = (unsigned char *)malloc(hmac_input_len);
    if (!hmac_input) goto cleanup;

    size_t off = 0;
    memcpy(hmac_input + off, MAGIC, MAGIC_LEN); off += MAGIC_LEN;
    hmac_input[off++] = (unsigned char)VERSION;
    memcpy(hmac_input + off, salt, SALT_LEN); off += SALT_LEN;
    memcpy(hmac_input + off, iv, IV_LEN); off += IV_LEN;
    memcpy(hmac_input + off, ciphertext, ciphertext_len); off += ciphertext_len;

    unsigned char calc_tag[TAG_LEN];
    int mac_ok = compute_hmac_sha256(hmac_input, hmac_input_len, mac_key, MAC_KEY_LEN, calc_tag);
    secure_memzero(hmac_input, hmac_input_len);
    free(hmac_input);
    if (!mac_ok) goto cleanup;

    if (!consttime_eq(calc_tag, tag, TAG_LEN)) {
        secure_memzero(calc_tag, sizeof(calc_tag));
        goto cleanup;
    }
    secure_memzero(calc_tag, sizeof(calc_tag));

    /* Decrypt */
    if (!aes256_cbc_decrypt(ciphertext, ciphertext_len, enc_key, iv, out_plain, out_plain_len)) {
        goto cleanup;
    }

    ok = 1;

cleanup:
    secure_memzero(enc_key, sizeof(enc_key));
    secure_memzero(mac_key, sizeof(mac_key));
    return ok;
}

/* ================= Test Harness (No sensitive data printed) ================= */
static int gen_random_bytes(unsigned char *buf, size_t len) {
    if (!buf || len == 0) return 0;
    return RAND_bytes(buf, (int)len) == 1 ? 1 : 0;
}

static void print_test_result(const char *name, int ok) {
    if (!name) return;
    /* Do not print any sensitive material. */
    if (ok) {
        printf("%s: OK\n", name);
    } else {
        printf("%s: FAILED\n", name);
    }
}

int main(void) {
    /* Initialize OpenSSL (for compatibility with older versions; OpenSSL 1.1+ auto-inits). */
    /* Tests avoid printing any secrets. */

    int ok = 1;

    /* Test 1: Roundtrip 32-byte key */
    {
        unsigned char key[32];
        unsigned char *pkg = NULL; size_t pkg_len = 0;
        unsigned char *plain = NULL; size_t plain_len = 0;
        const char *pass = "correct horse battery staple";

        ok = gen_random_bytes(key, sizeof(key));
        if (!ok) { print_test_result("Test 1 RNG", 0); return 1; }

        ok = encrypt_key_with_aes_cbc(key, sizeof(key), pass, strlen(pass), &pkg, &pkg_len);
        if (!ok) { print_test_result("Test 1 Encrypt", 0); return 1; }

        ok = decrypt_key_with_aes_cbc(pkg, pkg_len, pass, strlen(pass), &plain, &plain_len);
        if (!ok) { print_test_result("Test 1 Decrypt", 0); free(pkg); return 1; }

        int eq = (plain_len == sizeof(key)) && consttime_eq(plain, key, sizeof(key));
        print_test_result("Test 1 Roundtrip", eq);

        secure_memzero(key, sizeof(key));
        secure_memzero(plain, plain_len);
        free(plain);
        secure_memzero(pkg, pkg_len);
        free(pkg);
    }

    /* Test 2: Roundtrip 16-byte key */
    {
        unsigned char key[16];
        unsigned char *pkg = NULL; size_t pkg_len = 0;
        unsigned char *plain = NULL; size_t plain_len = 0;
        const char *pass = "s3cure passphrase 2";

        ok = gen_random_bytes(key, sizeof(key));
        if (!ok) { print_test_result("Test 2 RNG", 0); return 1; }

        ok = encrypt_key_with_aes_cbc(key, sizeof(key), pass, strlen(pass), &pkg, &pkg_len);
        if (!ok) { print_test_result("Test 2 Encrypt", 0); return 1; }

        ok = decrypt_key_with_aes_cbc(pkg, pkg_len, pass, strlen(pass), &plain, &plain_len);
        if (!ok) { print_test_result("Test 2 Decrypt", 0); free(pkg); return 1; }

        int eq = (plain_len == sizeof(key)) && consttime_eq(plain, key, sizeof(key));
        print_test_result("Test 2 Roundtrip", eq);

        secure_memzero(key, sizeof(key));
        secure_memzero(plain, plain_len);
        free(plain);
        secure_memzero(pkg, pkg_len);
        free(pkg);
    }

    /* Test 3: Roundtrip 48-byte key */
    {
        unsigned char key[48];
        unsigned char *pkg = NULL; size_t pkg_len = 0;
        unsigned char *plain = NULL; size_t plain_len = 0;
        const char *pass = "another strong passphrase";

        ok = gen_random_bytes(key, sizeof(key));
        if (!ok) { print_test_result("Test 3 RNG", 0); return 1; }

        ok = encrypt_key_with_aes_cbc(key, sizeof(key), pass, strlen(pass), &pkg, &pkg_len);
        if (!ok) { print_test_result("Test 3 Encrypt", 0); return 1; }

        ok = decrypt_key_with_aes_cbc(pkg, pkg_len, pass, strlen(pass), &plain, &plain_len);
        if (!ok) { print_test_result("Test 3 Decrypt", 0); free(pkg); return 1; }

        int eq = (plain_len == sizeof(key)) && consttime_eq(plain, key, sizeof(key));
        print_test_result("Test 3 Roundtrip", eq);

        secure_memzero(key, sizeof(key));
        secure_memzero(plain, plain_len);
        free(plain);
        secure_memzero(pkg, pkg_len);
        free(pkg);
    }

    /* Test 4: Wrong passphrase should fail (MAC or padding mismatch) */
    {
        unsigned char key[24];
        unsigned char *pkg = NULL; size_t pkg_len = 0;
        unsigned char *plain = NULL; size_t plain_len = 0;
        const char *pass = "the right passphrase";
        const char *wrong = "the wrong passphrase";

        ok = gen_random_bytes(key, sizeof(key));
        if (!ok) { print_test_result("Test 4 RNG", 0); return 1; }

        ok = encrypt_key_with_aes_cbc(key, sizeof(key), pass, strlen(pass), &pkg, &pkg_len);
        if (!ok) { print_test_result("Test 4 Encrypt", 0); return 1; }

        int dec_ok = decrypt_key_with_aes_cbc(pkg, pkg_len, wrong, strlen(wrong), &plain, &plain_len);
        print_test_result("Test 4 Wrong Passphrase (expected failure)", !dec_ok);

        if (dec_ok) {
            secure_memzero(plain, plain_len);
            free(plain);
        }
        secure_memzero(pkg, pkg_len);
        free(pkg);
        secure_memzero(key, sizeof(key));
    }

    /* Test 5: Tamper ciphertext to ensure MAC check fails */
    {
        unsigned char key[32];
        unsigned char *pkg = NULL; size_t pkg_len = 0;
        unsigned char *plain = NULL; size_t plain_len = 0;
        const char *pass = "tamper-resistance passphrase";

        ok = gen_random_bytes(key, sizeof(key));
        if (!ok) { print_test_result("Test 5 RNG", 0); return 1; }

        ok = encrypt_key_with_aes_cbc(key, sizeof(key), pass, strlen(pass), &pkg, &pkg_len);
        if (!ok) { print_test_result("Test 5 Encrypt", 0); return 1; }

        /* Tamper a byte in the ciphertext area (after header) */
        size_t header_len = MAGIC_LEN + VERSION_LEN + SALT_LEN + IV_LEN;
        if (pkg_len > header_len + TAG_LEN + 1) {
            pkg[header_len + 1] ^= 0x01;
        }

        int dec_ok = decrypt_key_with_aes_cbc(pkg, pkg_len, pass, strlen(pass), &plain, &plain_len);
        print_test_result("Test 5 Tamper Detect (expected failure)", !dec_ok);

        if (dec_ok) {
            secure_memzero(plain, plain_len);
            free(plain);
        }
        secure_memzero(pkg, pkg_len);
        free(pkg);
        secure_memzero(key, sizeof(key));
    }

    return 0;
}