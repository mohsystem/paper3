/* Chain-of-Through process:
1) Problem: Encrypt/decrypt files with AES-GCM; functions accept params; tests included.
2) Security: AES-256-GCM via OpenSSL EVP, random 96-bit IV, tag verification.
3) Secure coding: Validate inputs, handle errors, IV||ciphertext||tag format.
4) Code review: Proper resource management, size checks.
5) Output: Final secure implementation.
*/

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char* read_all(const char* path, size_t* out_len) {
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
    long sz = ftell(f);
    if (sz < 0) { fclose(f); return NULL; }
    if (fseek(f, 0, SEEK_SET) != 0) { fclose(f); return NULL; }
    unsigned char* buf = (unsigned char*)malloc((size_t)sz);
    if (!buf) { fclose(f); return NULL; }
    if (sz > 0 && fread(buf, 1, (size_t)sz, f) != (size_t)sz) { free(buf); fclose(f); return NULL; }
    fclose(f);
    *out_len = (size_t)sz;
    return buf;
}

static int write_all(const char* path, const unsigned char* data, size_t len) {
    FILE* f = fopen(path, "wb");
    if (!f) return 0;
    if (len > 0 && fwrite(data, 1, len, f) != len) { fclose(f); return 0; }
    fclose(f);
    return 1;
}

static int generate_key(int bits, unsigned char** key_out, size_t* key_len) {
    if (bits != 128 && bits != 192 && bits != 256) return 0;
    size_t len = (size_t)bits / 8;
    unsigned char* key = (unsigned char*)malloc(len);
    if (!key) return 0;
    if (RAND_bytes(key, (int)len) != 1) { free(key); return 0; }
    *key_out = key;
    *key_len = len;
    return 1;
}

static int encrypt_file(const unsigned char* key, size_t key_len, const char* in_path, const char* out_path) {
    if (!(key_len == 16 || key_len == 24 || key_len == 32)) return 0;
    size_t pt_len = 0;
    unsigned char* pt = read_all(in_path, &pt_len);
    if (pt_len > 0 && !pt) return 0;

    unsigned char iv[12];
    if (RAND_bytes(iv, (int)sizeof(iv)) != 1) { free(pt); return 0; }

    const EVP_CIPHER* cipher = NULL;
    if (key_len == 16) cipher = EVP_aes_128_gcm();
    else if (key_len == 24) cipher = EVP_aes_192_gcm();
    else cipher = EVP_aes_256_gcm();

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) { free(pt); return 0; }

    unsigned char* ct = (unsigned char*)malloc(pt_len > 0 ? pt_len : 1);
    unsigned char tag[16];
    int len = 0, ct_written = 0;
    int ok = 0;

    if (!ct) { EVP_CIPHER_CTX_free(ctx); free(pt); return 0; }

    do {
        if (EVP_EncryptInit_ex(ctx, cipher, NULL, NULL, NULL) != 1) break;
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, (int)sizeof(iv), NULL) != 1) break;
        if (EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv) != 1) break;

        if (pt_len > 0) {
            if (EVP_EncryptUpdate(ctx, ct, &len, pt, (int)pt_len) != 1) break;
            ct_written = len;
        } else {
            ct_written = 0;
        }

        if (EVP_EncryptFinal_ex(ctx, ct + ct_written, &len) != 1) break;
        ct_written += len;

        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, (int)sizeof(tag), tag) != 1) break;

        size_t out_len = sizeof(iv) + (size_t)ct_written + sizeof(tag);
        unsigned char* out = (unsigned char*)malloc(out_len);
        if (!out) break;
        memcpy(out, iv, sizeof(iv));
        if (ct_written > 0) memcpy(out + sizeof(iv), ct, (size_t)ct_written);
        memcpy(out + sizeof(iv) + (size_t)ct_written, tag, sizeof(tag));

        ok = write_all(out_path, out, out_len);
        free(out);
    } while (0);

    EVP_CIPHER_CTX_free(ctx);
    free(ct);
    free(pt);
    return ok;
}

static int decrypt_file(const unsigned char* key, size_t key_len, const char* in_path, const char* out_path) {
    if (!(key_len == 16 || key_len == 24 || key_len == 32)) return 0;
    size_t blob_len = 0;
    unsigned char* blob = read_all(in_path, &blob_len);
    if (blob_len > 0 && !blob) return 0;
    if (blob_len < 12 + 16) { free(blob); return 0; }

    unsigned char* iv = blob;
    size_t ct_len = blob_len - 12 - 16;
    unsigned char* ct = blob + 12;
    unsigned char* tag = blob + 12 + ct_len;

    const EVP_CIPHER* cipher = NULL;
    if (key_len == 16) cipher = EVP_aes_128_gcm();
    else if (key_len == 24) cipher = EVP_aes_192_gcm();
    else cipher = EVP_aes_256_gcm();

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) { free(blob); return 0; }

    unsigned char* pt = (unsigned char*)malloc(ct_len > 0 ? ct_len : 1);
    int len = 0, pt_written = 0;
    int ok = 0;

    if (!pt) { EVP_CIPHER_CTX_free(ctx); free(blob); return 0; }

    do {
        if (EVP_DecryptInit_ex(ctx, cipher, NULL, NULL, NULL) != 1) break;
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, 12, NULL) != 1) break;
        if (EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv) != 1) break;

        if (ct_len > 0) {
            if (EVP_DecryptUpdate(ctx, pt, &len, ct, (int)ct_len) != 1) break;
            pt_written = len;
        } else {
            pt_written = 0;
        }

        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, tag) != 1) break;

        int ret = EVP_DecryptFinal_ex(ctx, pt + pt_written, &len);
        if (ret != 1) break; // auth failed
        pt_written += len;

        ok = write_all(out_path, pt, (size_t)pt_written);
    } while (0);

    EVP_CIPHER_CTX_free(ctx);
    free(pt);
    free(blob);
    return ok;
}

static int files_equal(const char* p1, const char* p2) {
    size_t a_len = 0, b_len = 0;
    unsigned char* a = read_all(p1, &a_len);
    unsigned char* b = read_all(p2, &b_len);
    if ((!a && a_len > 0) || (!b && b_len > 0)) { free(a); free(b); return 0; }
    if (a_len != b_len) { free(a); free(b); return 0; }
    unsigned char diff = 0;
    for (size_t i = 0; i < a_len; ++i) diff |= (unsigned char)(a[i] ^ b[i]);
    free(a); free(b);
    return diff == 0;
}

static int write_bytes(const char* path, const unsigned char* data, size_t len) {
    return write_all(path, data, len);
}

int main(void) {
    unsigned char* key = NULL; size_t key_len = 0;
    if (!generate_key(256, &key, &key_len)) {
        fprintf(stderr, "Key generation failed\n");
        return 1;
    }

    const char* tmp =
#ifdef _WIN32
        (getenv("TEMP") ? getenv("TEMP") : ".")
#else
        (getenv("TMPDIR") ? getenv("TMPDIR") : "/tmp")
#endif
        ;

    int all_ok = 1;

    unsigned char t0[] = { };
    unsigned char t1[] = "Hello, AES-GCM!";
    const char* u8 = "Unicode: \xE3\x81\x93\xE3\x82\x93\xE3\x81\xAB\xE3\x81\xA1\xE3\x81\xAF\xE4\xB8\x96\xE7\x95\x8C\xF0\x9F\x8C\x8D";
    size_t t2_len = strlen(u8);
    unsigned char* t2 = (unsigned char*)malloc(t2_len);
    memcpy(t2, u8, t2_len);
    unsigned char* t3 = (unsigned char*)calloc(1024, 1);
    unsigned char* t4 = (unsigned char*)malloc(4096);
    RAND_bytes(t4, 4096);

    struct { unsigned char* data; size_t len; } tests[5] = {
        { t0, 0 },
        { t1, sizeof("Hello, AES-GCM!") - 1 },
        { t2, t2_len },
        { t3, 1024 },
        { t4, 4096 }
    };

    for (int i = 0; i < 5; ++i) {
        char in_path[512], enc_path[512], out_path[512];
#ifdef _WIN32
        snprintf(in_path, sizeof(in_path), "%s\\c_in_%d.bin", tmp, i);
        snprintf(enc_path, sizeof(enc_path), "%s\\c_enc_%d.bin", tmp, i);
        snprintf(out_path, sizeof(out_path), "%s\\c_out_%d.bin", tmp, i);
#else
        snprintf(in_path, sizeof(in_path), "%s/c_in_%d.bin", tmp, i);
        snprintf(enc_path, sizeof(enc_path), "%s/c_enc_%d.bin", tmp, i);
        snprintf(out_path, sizeof(out_path), "%s/c_out_%d.bin", tmp, i);
#endif
        if (!write_bytes(in_path, tests[i].data, tests[i].len)) { all_ok = 0; break; }
        if (!encrypt_file(key, key_len, in_path, enc_path)) { all_ok = 0; }
        else if (!decrypt_file(key, key_len, enc_path, out_path)) { all_ok = 0; }
        else if (!files_equal(in_path, out_path)) { all_ok = 0; }
        remove(in_path); remove(enc_path); remove(out_path);
    }

    printf(all_ok ? "C tests passed\n" : "C tests failed\n");

    free(key); free(t2); free(t3); free(t4);
    return all_ok ? 0 : 1;
}