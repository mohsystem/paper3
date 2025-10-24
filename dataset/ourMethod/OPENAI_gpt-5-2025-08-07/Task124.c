#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>
#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define MKDIR(path) mkdir(path, 0700)
#endif

#define MAGIC "ENC1"
#define VERSION 1
#define SALT_LEN 16
#define IV_LEN 12
#define TAG_LEN 16
#define KEY_LEN 32
#define PBKDF2_ITERS 210000

typedef struct {
    uint8_t* data;
    size_t len;
} Buffer;

static void buffer_free(Buffer* b) {
    if (b && b->data) {
        OPENSSL_cleanse(b->data, b->len);
        free(b->data);
        b->data = NULL;
        b->len = 0;
    }
}

static int derive_key(const char* passphrase, const uint8_t* salt, uint8_t* out_key, size_t out_len) {
    if (!passphrase || !salt || !out_key) return 0;
    return PKCS5_PBKDF2_HMAC(passphrase, (int)strlen(passphrase), salt, SALT_LEN, PBKDF2_ITERS, EVP_sha256(), (int)out_len, out_key) == 1;
}

static Buffer encrypt_to_bytes(const char* passphrase, const uint8_t* data, size_t datalen) {
    Buffer out = {NULL, 0};
    if (!passphrase || (!data && datalen != 0)) return out;

    uint8_t salt[SALT_LEN];
    uint8_t iv[IV_LEN];
    if (RAND_bytes(salt, SALT_LEN) != 1 || RAND_bytes(iv, IV_LEN) != 1) return out;

    uint8_t key[KEY_LEN];
    if (!derive_key(passphrase, salt, key, KEY_LEN)) {
        OPENSSL_cleanse(key, KEY_LEN);
        return out;
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        OPENSSL_cleanse(key, KEY_LEN);
        return out;
    }

    uint8_t* ciphertext = NULL;
    int len = 0, outlen = 0;
    int ok = 1;

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) ok = 0;
    if (ok && EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, IV_LEN, NULL) != 1) ok = 0;
    if (ok && EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv) != 1) ok = 0;

    ciphertext = (uint8_t*)malloc(datalen > 0 ? datalen : 1);
    if (!ciphertext) ok = 0;

    if (ok && datalen > 0) {
        if (EVP_EncryptUpdate(ctx, ciphertext, &len, data, (int)datalen) != 1) ok = 0;
        outlen = len;
    } else {
        outlen = 0;
    }
    if (ok && EVP_EncryptFinal_ex(ctx, ciphertext + outlen, &len) != 1) ok = 0;
    outlen += len;

    uint8_t tag[TAG_LEN];
    if (ok && EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_LEN, tag) != 1) ok = 0;

    if (!ok) {
        if (ciphertext) { OPENSSL_cleanse(ciphertext, datalen); free(ciphertext); }
        EVP_CIPHER_CTX_free(ctx);
        OPENSSL_cleanse(key, KEY_LEN);
        return out;
    }

    size_t total = 4 + 1 + SALT_LEN + IV_LEN + (size_t)outlen + TAG_LEN;
    uint8_t* buf = (uint8_t*)malloc(total);
    if (!buf) {
        OPENSSL_cleanse(ciphertext, outlen);
        free(ciphertext);
        EVP_CIPHER_CTX_free(ctx);
        OPENSSL_cleanse(key, KEY_LEN);
        return out;
    }

    uint8_t* p = buf;
    memcpy(p, MAGIC, 4); p += 4;
    *p++ = VERSION;
    memcpy(p, salt, SALT_LEN); p += SALT_LEN;
    memcpy(p, iv, IV_LEN); p += IV_LEN;
    memcpy(p, ciphertext, (size_t)outlen); p += outlen;
    memcpy(p, tag, TAG_LEN); p += TAG_LEN;

    out.data = buf;
    out.len = total;

    OPENSSL_cleanse(ciphertext, outlen);
    free(ciphertext);
    EVP_CIPHER_CTX_free(ctx);
    OPENSSL_cleanse(key, KEY_LEN);
    return out;
}

static Buffer decrypt_from_bytes(const char* passphrase, const uint8_t* blob, size_t bloblen) {
    Buffer out = {NULL, 0};
    if (!passphrase || !blob) return out;
    size_t minLen = 4 + 1 + SALT_LEN + IV_LEN + TAG_LEN;
    if (bloblen < minLen) return out;
    if (memcmp(blob, MAGIC, 4) != 0) return out;
    if (blob[4] != VERSION) return out;

    const uint8_t* salt = blob + 5;
    const uint8_t* iv = blob + 5 + SALT_LEN;
    const uint8_t* rest = blob + 5 + SALT_LEN + IV_LEN;
    size_t restLen = bloblen - (5 + SALT_LEN + IV_LEN);
    if (restLen < TAG_LEN) return out;

    size_t ctLen = restLen - TAG_LEN;
    const uint8_t* ciphertext = rest;
    const uint8_t* tag = rest + ctLen;

    uint8_t key[KEY_LEN];
    if (!derive_key(passphrase, salt, key, KEY_LEN)) {
        OPENSSL_cleanse(key, KEY_LEN);
        return out;
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        OPENSSL_cleanse(key, KEY_LEN);
        return out;
    }

    uint8_t* plaintext = (uint8_t*)malloc(ctLen > 0 ? ctLen : 1);
    if (!plaintext) {
        EVP_CIPHER_CTX_free(ctx);
        OPENSSL_cleanse(key, KEY_LEN);
        return out;
    }

    int len = 0, outlen = 0;
    int ok = 1;

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) ok = 0;
    if (ok && EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, IV_LEN, NULL) != 1) ok = 0;
    if (ok && EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv) != 1) ok = 0;
    if (ok && ctLen > 0) {
        if (EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, (int)ctLen) != 1) ok = 0;
        outlen = len;
    } else {
        outlen = 0;
    }
    if (ok && EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, TAG_LEN, (void*)tag) != 1) ok = 0;
    if (ok && EVP_DecryptFinal_ex(ctx, plaintext + outlen, &len) != 1) ok = 0;
    outlen += len;

    if (!ok) {
        OPENSSL_cleanse(plaintext, ctLen);
        free(plaintext);
        EVP_CIPHER_CTX_free(ctx);
        OPENSSL_cleanse(key, KEY_LEN);
        return out;
    }

    out.data = plaintext;
    out.len = (size_t)outlen;

    EVP_CIPHER_CTX_free(ctx);
    OPENSSL_cleanse(key, KEY_LEN);
    return out;
}

static int ensure_dirs(const char* path) {
    // Create directories recursively (simple approach)
    char tmp[1024];
    size_t len = strlen(path);
    if (len >= sizeof(tmp)) return 0;
    strncpy(tmp, path, sizeof(tmp));
    tmp[sizeof(tmp)-1] = '\0';
    for (size_t i = 0; i < len; ++i) {
        if (tmp[i] == '/' || tmp[i] == '\\') {
            char c = tmp[i];
            tmp[i] = '\0';
            if (strlen(tmp) > 0) {
                MKDIR(tmp);
            }
            tmp[i] = c;
        }
    }
    return 1;
}

static int is_path_safe(const char* base_dir, const char* user_rel, char* out_path, size_t out_size) {
    if (!base_dir || !user_rel || !out_path) return 0;
    if (user_rel[0] == '/' || user_rel[0] == '\\') return 0;
    if (strstr(user_rel, "..") != NULL) return 0;
    int n = snprintf(out_path, out_size, "%s/%s", base_dir, user_rel);
    if (n <= 0 || (size_t)n >= out_size) return 0;
    return 1;
}

static int save_encrypted(const char* base_dir, const char* user_rel, const char* passphrase, const uint8_t* data, size_t datalen) {
    char target[1024];
    if (!is_path_safe(base_dir, user_rel, target, sizeof(target))) return 0;

    Buffer blob = encrypt_to_bytes(passphrase, data, datalen);
    if (!blob.data || blob.len == 0) return 0;

    // Ensure directories exist
    char dirpath[1024];
    strncpy(dirpath, target, sizeof(dirpath));
    dirpath[sizeof(dirpath)-1] = '\0';
    for (char* p = dirpath + strlen(dirpath); p != dirpath; --p) {
        if (*p == '/' || *p == '\\') { *p = '\0'; break; }
    }
    if (strlen(dirpath) > 0) ensure_dirs(dirpath);

    char tmpfile[1060];
    snprintf(tmpfile, sizeof(tmpfile), "%s.tmp", target);

    FILE* f = fopen(tmpfile, "wb");
    if (!f) {
        buffer_free(&blob);
        return 0;
    }
    size_t wrote = fwrite(blob.data, 1, blob.len, f);
    fflush(f);
    fclose(f);
    buffer_free(&blob);
    if (wrote != blob.len) {
        remove(tmpfile);
        return 0;
    }
    // Atomic replace where possible
    if (rename(tmpfile, target) != 0) {
        remove(target);
        if (rename(tmpfile, target) != 0) {
            remove(tmpfile);
            return 0;
        }
    }
    return 1;
}

static Buffer load_decrypted(const char* base_dir, const char* user_rel, const char* passphrase) {
    Buffer out = {NULL, 0};
    char target[1024];
    if (!is_path_safe(base_dir, user_rel, target, sizeof(target))) return out;

    FILE* f = fopen(target, "rb");
    if (!f) return out;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return out; }
    long sz = ftell(f);
    if (sz < 0) { fclose(f); return out; }
    rewind(f);
    uint8_t* buf = (uint8_t*)malloc((size_t)sz);
    if (!buf) { fclose(f); return out; }
    size_t rd = fread(buf, 1, (size_t)sz, f);
    fclose(f);
    if (rd != (size_t)sz) { free(buf); return out; }
    Buffer dec = decrypt_from_bytes(passphrase, buf, (size_t)sz);
    OPENSSL_cleanse(buf, (size_t)sz);
    free(buf);
    return dec;
}

static void random_passphrase(char* out, size_t outlen, size_t nbytes) {
    uint8_t* tmp = (uint8_t*)malloc(nbytes);
    if (!tmp) { if (outlen>0) out[0]='\0'; return; }
    RAND_bytes(tmp, (int)nbytes);
    static const char* hex = "0123456789abcdef";
    size_t need = nbytes * 2 + 1;
    if (outlen < need) { free(tmp); if (outlen>0) out[0]='\0'; return; }
    for (size_t i = 0; i < nbytes; ++i) {
        out[i*2] = hex[(tmp[i]>>4)&0xF];
        out[i*2+1] = hex[tmp[i]&0xF];
    }
    out[nbytes*2] = '\0';
    OPENSSL_cleanse(tmp, nbytes);
    free(tmp);
}

int main(void) {
    const char* base = "secure_store_c";
    const char* files[5] = {"rec1.bin","rec2.bin","rec3.bin","rec4.bin","rec5.bin"};
    const char* records[5] = {
        "User: Alice; CC: 4111-1111-1111-1111; Exp: 12/29",
        "User: Bob; SSN: 123-45-6789; DOB: 1990-01-01",
        "User: Carol; Passport: X12345678; Country: US",
        "User: Dave; Bank: 0123456789; Routing: 021000021",
        "User: Eve; Email: eve@example.com; Phone: +1-555-0100"
    };
    char pass[5][65];
    for (int i = 0; i < 5; ++i) {
        random_passphrase(pass[i], sizeof(pass[i]), 16);
    }
    for (int i = 0; i < 5; ++i) {
        const uint8_t* data = (const uint8_t*)records[i];
        size_t len = strlen(records[i]);
        int ok = save_encrypted(base, files[i], pass[i], data, len);
        if (!ok) {
            printf("Test %d save failed\n", i+1);
            continue;
        }
        Buffer dec = load_decrypted(base, files[i], pass[i]);
        if (dec.data && dec.len == len && memcmp(dec.data, data, len) == 0) {
            printf("Test %d OK\n", i+1);
        } else {
            printf("Test %d mismatch\n", i+1);
        }
        buffer_free(&dec);
    }
    return 0;
}