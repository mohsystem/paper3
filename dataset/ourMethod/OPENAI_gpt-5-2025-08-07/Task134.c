#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <openssl/err.h>

#define MAGIC "ENC1"
#define MAGIC_LEN 4
#define VERSION 1
#define SALT_LEN 16
#define IV_LEN 12
#define TAG_LEN 16
#define PBKDF2_ITERS 210000
#define KEY_LEN 32

typedef struct {
    unsigned char* data;
    size_t len;
} Buffer;

typedef struct {
    EVP_PKEY* priv;
    EVP_PKEY* pub;
} RSAKeyPair;

static void buffer_free(Buffer b) {
    if (b.data) {
        OPENSSL_cleanse(b.data, b.len);
        free(b.data);
    }
}

static Buffer buffer_new(size_t len) {
    Buffer b;
    b.data = (unsigned char*)malloc(len ? len : 1);
    b.len = (b.data ? len : 0);
    return b;
}

static int secure_random(unsigned char* out, size_t len) {
    if (!out || len == 0) return 0;
    return RAND_bytes(out, (int)len) == 1;
}

static int derive_key(const char* passphrase, const unsigned char* salt, size_t salt_len, int iterations, unsigned char* out_key, size_t out_len) {
    if (!passphrase || !salt || !out_key || salt_len != SALT_LEN || out_len != KEY_LEN) return 0;
    return PKCS5_PBKDF2_HMAC(passphrase, (int)strlen(passphrase), salt, (int)salt_len, iterations, EVP_sha256(), (int)out_len, out_key) == 1;
}

static Buffer encrypt_aes_gcm(const unsigned char* plaintext, size_t pt_len, const char* passphrase) {
    Buffer out = {NULL, 0};
    if (!plaintext || !passphrase) return out;

    unsigned char salt[SALT_LEN];
    unsigned char iv[IV_LEN];
    if (!secure_random(salt, SALT_LEN) || !secure_random(iv, IV_LEN)) return out;

    unsigned char key[KEY_LEN];
    if (!derive_key(passphrase, salt, SALT_LEN, PBKDF2_ITERS, key, KEY_LEN)) return out;

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) { OPENSSL_cleanse(key, KEY_LEN); return out; }

    int ok = 1;
    int len = 0, total = 0;
    unsigned char* ciphertext = (unsigned char*)malloc(pt_len + 16);
    if (!ciphertext) { ok = 0; }
    if (ok && EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) ok = 0;
    if (ok && EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, IV_LEN, NULL) != 1) ok = 0;
    if (ok && EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv) != 1) ok = 0;
    if (ok && pt_len > 0) {
        if (EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, (int)pt_len) != 1) ok = 0;
        total = len;
    } else {
        if (EVP_EncryptUpdate(ctx, NULL, &len, NULL, 0) != 1) ok = 0;
    }
    if (ok && EVP_EncryptFinal_ex(ctx, ciphertext + total, &len) != 1) ok = 0;
    total += len;
    unsigned char tag[TAG_LEN];
    if (ok && EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_LEN, tag) != 1) ok = 0;

    if (ok) {
        size_t out_len = MAGIC_LEN + 1 + SALT_LEN + IV_LEN + (size_t)total + TAG_LEN;
        out = buffer_new(out_len);
        if (out.data) {
            size_t off = 0;
            memcpy(out.data + off, MAGIC, MAGIC_LEN); off += MAGIC_LEN;
            out.data[off++] = VERSION;
            memcpy(out.data + off, salt, SALT_LEN); off += SALT_LEN;
            memcpy(out.data + off, iv, IV_LEN); off += IV_LEN;
            memcpy(out.data + off, ciphertext, (size_t)total); off += (size_t)total;
            memcpy(out.data + off, tag, TAG_LEN); off += TAG_LEN;
        } else {
            out.len = 0;
        }
    }

    if (ciphertext) {
        OPENSSL_cleanse(ciphertext, (size_t)total);
        free(ciphertext);
    }
    EVP_CIPHER_CTX_free(ctx);
    OPENSSL_cleanse(key, KEY_LEN);
    if (!ok) {
        buffer_free(out);
        out.data = NULL;
        out.len = 0;
    }
    return out;
}

static Buffer decrypt_aes_gcm(const unsigned char* data, size_t data_len, const char* passphrase) {
    Buffer out = {NULL, 0};
    if (!data || !passphrase) return out;
    if (data_len < MAGIC_LEN + 1 + SALT_LEN + IV_LEN + TAG_LEN) return out;
    if (CRYPTO_memcmp(data, MAGIC, MAGIC_LEN) != 0) return out;
    if (data[MAGIC_LEN] != VERSION) return out;

    size_t offset = MAGIC_LEN + 1;
    const unsigned char* salt = data + offset; offset += SALT_LEN;
    const unsigned char* iv = data + offset; offset += IV_LEN;
    if (data_len < offset + TAG_LEN) return out;
    size_t ct_len = data_len - offset - TAG_LEN;
    const unsigned char* ct = data + offset;
    const unsigned char* tag = data + offset + ct_len;

    unsigned char key[KEY_LEN];
    if (!derive_key(passphrase, salt, SALT_LEN, PBKDF2_ITERS, key, KEY_LEN)) return out;

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) { OPENSSL_cleanse(key, KEY_LEN); return out; }

    int ok = 1;
    int len = 0, total = 0;
    unsigned char* plaintext = (unsigned char*)malloc(ct_len ? ct_len : 1);
    if (!plaintext) ok = 0;

    if (ok && EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) ok = 0;
    if (ok && EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, IV_LEN, NULL) != 1) ok = 0;
    if (ok && EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv) != 1) ok = 0;
    if (ok && ct_len > 0) {
        if (EVP_DecryptUpdate(ctx, plaintext, &len, ct, (int)ct_len) != 1) ok = 0;
        total = len;
    }
    if (ok && EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, TAG_LEN, (void*)tag) != 1) ok = 0;
    if (ok && EVP_DecryptFinal_ex(ctx, plaintext + total, &len) != 1) ok = 0;
    total += len;

    if (ok) {
        out = buffer_new((size_t)total);
        if (out.data && total > 0) {
            memcpy(out.data, plaintext, (size_t)total);
        } else if (total == 0) {
            // Empty plaintext is valid
        } else {
            out.len = 0;
        }
        out.len = (size_t)total;
    }

    if (plaintext) {
        OPENSSL_cleanse(plaintext, ct_len);
        free(plaintext);
    }
    EVP_CIPHER_CTX_free(ctx);
    OPENSSL_cleanse(key, KEY_LEN);

    if (!ok) {
        buffer_free(out);
        out.data = NULL;
        out.len = 0;
    }
    return out;
}

static RSAKeyPair generate_rsa_keypair(int bits) {
    RSAKeyPair kp;
    kp.priv = NULL;
    kp.pub = NULL;

    EVP_PKEY_CTX* kctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    if (!kctx) return kp;
    if (EVP_PKEY_keygen_init(kctx) != 1) { EVP_PKEY_CTX_free(kctx); return kp; }
    if (EVP_PKEY_CTX_set_rsa_keygen_bits(kctx, bits) != 1) { EVP_PKEY_CTX_free(kctx); return kp; }
    EVP_PKEY* priv = NULL;
    if (EVP_PKEY_keygen(kctx, &priv) != 1) { EVP_PKEY_CTX_free(kctx); return kp; }
    EVP_PKEY_CTX_free(kctx);
    kp.priv = priv;

    RSA* rsa = EVP_PKEY_get1_RSA(priv);
    if (!rsa) { EVP_PKEY_free(priv); kp.priv = NULL; return kp; }
    RSA* rsa_pub = RSAPublicKey_dup(rsa);
    RSA_free(rsa);
    if (!rsa_pub) { EVP_PKEY_free(priv); kp.priv = NULL; return kp; }
    EVP_PKEY* pub = EVP_PKEY_new();
    if (!pub) { RSA_free(rsa_pub); EVP_PKEY_free(priv); kp.priv = NULL; return kp; }
    if (EVP_PKEY_assign_RSA(pub, rsa_pub) != 1) { RSA_free(rsa_pub); EVP_PKEY_free(pub); EVP_PKEY_free(priv); kp.priv = NULL; return kp; }
    kp.pub = pub;

    return kp;
}

static Buffer rsa_encrypt(const unsigned char* plaintext, size_t pt_len, EVP_PKEY* pub) {
    Buffer out = {NULL, 0};
    if (!plaintext || !pub) return out;
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pub, NULL);
    if (!ctx) return out;
    if (EVP_PKEY_encrypt_init(ctx) != 1) { EVP_PKEY_CTX_free(ctx); return out; }
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) != 1) { EVP_PKEY_CTX_free(ctx); return out; }
    if (EVP_PKEY_CTX_set_rsa_oaep_md(ctx, EVP_sha256()) != 1) { EVP_PKEY_CTX_free(ctx); return out; }
    if (EVP_PKEY_CTX_set_rsa_mgf1_md(ctx, EVP_sha256()) != 1) { EVP_PKEY_CTX_free(ctx); return out; }

    size_t outlen = 0;
    if (EVP_PKEY_encrypt(ctx, NULL, &outlen, plaintext, pt_len) != 1) { EVP_PKEY_CTX_free(ctx); return out; }
    out = buffer_new(outlen);
    size_t tmplen = outlen;
    if (!out.data || EVP_PKEY_encrypt(ctx, out.data, &tmplen, plaintext, pt_len) != 1) {
        buffer_free(out);
        out.data = NULL;
        out.len = 0;
        EVP_PKEY_CTX_free(ctx);
        return out;
    }
    out.len = tmplen;
    EVP_PKEY_CTX_free(ctx);
    return out;
}

static Buffer rsa_decrypt(const unsigned char* ciphertext, size_t ct_len, EVP_PKEY* priv) {
    Buffer out = {NULL, 0};
    if (!ciphertext || !priv) return out;
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(priv, NULL);
    if (!ctx) return out;
    if (EVP_PKEY_decrypt_init(ctx) != 1) { EVP_PKEY_CTX_free(ctx); return out; }
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) != 1) { EVP_PKEY_CTX_free(ctx); return out; }
    if (EVP_PKEY_CTX_set_rsa_oaep_md(ctx, EVP_sha256()) != 1) { EVP_PKEY_CTX_free(ctx); return out; }
    if (EVP_PKEY_CTX_set_rsa_mgf1_md(ctx, EVP_sha256()) != 1) { EVP_PKEY_CTX_free(ctx); return out; }

    size_t outlen = 0;
    if (EVP_PKEY_decrypt(ctx, NULL, &outlen, ciphertext, ct_len) != 1) { EVP_PKEY_CTX_free(ctx); return out; }
    out = buffer_new(outlen);
    size_t tmplen = outlen;
    if (!out.data || EVP_PKEY_decrypt(ctx, out.data, &tmplen, ciphertext, ct_len) != 1) {
        buffer_free(out);
        out.data = NULL;
        out.len = 0;
        EVP_PKEY_CTX_free(ctx);
        return out;
    }
    out.len = tmplen;
    EVP_PKEY_CTX_free(ctx);
    return out;
}

static int ct_equal(const unsigned char* a, const unsigned char* b, size_t len) {
    unsigned char r = 0;
    for (size_t i = 0; i < len; i++) r |= (a[i] ^ b[i]);
    return r == 0;
}

int main(void) {
    int all = 1;

    // Test 1: AES-GCM roundtrip
    const unsigned char msg1[] = "Secret message 1";
    const char* pw1 = "correct horse battery staple";
    Buffer enc1 = encrypt_aes_gcm(msg1, sizeof(msg1) - 1, pw1);
    Buffer dec1 = decrypt_aes_gcm(enc1.data, enc1.len, pw1);
    int t1 = (dec1.data != NULL && dec1.len == (sizeof(msg1) - 1) && ct_equal(dec1.data, msg1, dec1.len));
    printf("Test1 AES roundtrip: %s\n", t1 ? "true" : "false");
    all &= t1;

    // Test 2: AES wrong passphrase
    Buffer dec2 = decrypt_aes_gcm(enc1.data, enc1.len, "wrong pass");
    int t2 = (dec2.data == NULL);
    printf("Test2 AES wrong passphrase: %s\n", t2 ? "true" : "false");
    all &= t2;
    buffer_free(dec2);

    // Test 3: AES tamper detection
    Buffer tampered = buffer_new(enc1.len);
    if (tampered.data && enc1.data) {
        memcpy(tampered.data, enc1.data, enc1.len);
        size_t payloadStart = MAGIC_LEN + 1 + SALT_LEN + IV_LEN;
        if (tampered.len > payloadStart) {
            tampered.data[payloadStart] ^= 0x01;
        }
    }
    Buffer dec3 = decrypt_aes_gcm(tampered.data, tampered.len, pw1);
    int t3 = (dec3.data == NULL);
    printf("Test3 AES tamper detection: %s\n", t3 ? "true" : "false");
    all &= t3;
    buffer_free(dec3);
    buffer_free(tampered);

    // Test 4: RSA OAEP roundtrip
    RSAKeyPair kp = generate_rsa_keypair(2048);
    const unsigned char msg2[] = "RSA secret";
    Buffer renc = rsa_encrypt(msg2, sizeof(msg2) - 1, kp.pub);
    Buffer rdec = rsa_decrypt(renc.data, renc.len, kp.priv);
    int t4 = (rdec.data != NULL && rdec.len == (sizeof(msg2) - 1) && ct_equal(rdec.data, msg2, rdec.len));
    printf("Test4 RSA OAEP roundtrip: %s\n", t4 ? "true" : "false");
    all &= t4;

    // Test 5: RSA wrong key fails
    RSAKeyPair kp2 = generate_rsa_keypair(2048);
    Buffer rdec_wrong = rsa_decrypt(renc.data, renc.len, kp2.priv);
    int t5 = (rdec_wrong.data == NULL);
    printf("Test5 RSA wrong key fails: %s\n", t5 ? "true" : "false");
    all &= t5;

    printf("All tests passed: %s\n", all ? "true" : "false");

    // Cleanup
    buffer_free(enc1);
    buffer_free(dec1);
    buffer_free(renc);
    buffer_free(rdec);
    buffer_free(rdec_wrong);

    if (kp.priv) EVP_PKEY_free(kp.priv);
    if (kp.pub) EVP_PKEY_free(kp.pub);
    if (kp2.priv) EVP_PKEY_free(kp2.priv);
    if (kp2.pub) EVP_PKEY_free(kp2.pub);

    return all ? 0 : 1;
}