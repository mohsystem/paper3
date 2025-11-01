#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

#define ITERATIONS 210000
#define SALT_LEN 16
#define DK_LEN 32
#define MIN_USERNAME_LEN 3
#define MAX_USERNAME_LEN 32
#define MIN_PASSWORD_LEN 12
#define MAX_PASSWORD_LEN 128

static int is_valid_username(const char* u) {
    if (u == NULL) return 0;
    size_t len = strlen(u);
    if (len < MIN_USERNAME_LEN || len > MAX_USERNAME_LEN) return 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)u[i];
        int ok = (c >= 'A' && c <= 'Z') ||
                 (c >= 'a' && c <= 'z') ||
                 (c >= '0' && c <= '9') ||
                 c == '_' || c == '.' || c == '-';
        if (!ok) return 0;
    }
    return 1;
}

static int is_valid_password(const char* p) {
    if (p == NULL) return 0;
    size_t len = strlen(p);
    if (len < MIN_PASSWORD_LEN || len > MAX_PASSWORD_LEN) return 0;
    int hasU = 0, hasL = 0, hasD = 0, hasS = 0;
    const char* specials = "!@#$%^&*()_+-=[]{};':\"\\|,.<>/?";
    for (size_t i = 0; p[i] != '\0'; i++) {
        unsigned char c = (unsigned char)p[i];
        if (c >= 'A' && c <= 'Z') hasU = 1;
        else if (c >= 'a' && c <= 'z') hasL = 1;
        else if (c >= '0' && c <= '9') hasD = 1;
        else if (strchr(specials, (int)c) != NULL) hasS = 1;
    }
    return hasU && hasL && hasD && hasS;
}

static size_t b64_len(size_t n) {
    return 4 * ((n + 2) / 3);
}

char* register_user(const char* username, const char* password) {
    if (!is_valid_username(username)) {
        return NULL;
    }
    if (!is_valid_password(password)) {
        return NULL;
    }

    unsigned char salt[SALT_LEN];
    if (RAND_bytes(salt, SALT_LEN) != 1) {
        return NULL;
    }

    unsigned char dk[DK_LEN];
    if (PKCS5_PBKDF2_HMAC(password, (int)strlen(password), salt, SALT_LEN,
                          ITERATIONS, EVP_sha256(), DK_LEN, dk) != 1) {
        return NULL;
    }

    size_t salt_b64_size = b64_len(SALT_LEN);
    size_t hash_b64_size = b64_len(DK_LEN);

    char* salt_b64 = (char*)malloc(salt_b64_size + 1);
    char* hash_b64 = (char*)malloc(hash_b64_size + 1);
    if (!salt_b64 || !hash_b64) {
        free(salt_b64);
        free(hash_b64);
        OPENSSL_cleanse(dk, DK_LEN);
        return NULL;
    }

    int s_written = EVP_EncodeBlock((unsigned char*)salt_b64, salt, SALT_LEN);
    int h_written = EVP_EncodeBlock((unsigned char*)hash_b64, dk, DK_LEN);
    if (s_written < 0 || h_written < 0) {
        free(salt_b64);
        free(hash_b64);
        OPENSSL_cleanse(dk, DK_LEN);
        return NULL;
    }
    salt_b64[s_written] = '\0';
    hash_b64[h_written] = '\0';

    const char* prefix = "pbkdf2_sha256$";
    char iter_buf[16];
    snprintf(iter_buf, sizeof(iter_buf), "%d", ITERATIONS);

    size_t prefix_len = strlen(prefix);
    size_t iter_len = strlen(iter_buf);
    size_t total_len;
    int overflow = 0;

    // Calculate total length safely: prefix + iterations + 1 + salt + 1 + hash + null
    // Check for overflows step by step
    size_t tmp = prefix_len;
    if (tmp > SIZE_MAX - iter_len) overflow = 1; else tmp += iter_len;
    if (!overflow && tmp > SIZE_MAX - 1) overflow = 1; else tmp += 1; // $
    if (!overflow && tmp > SIZE_MAX - (size_t)s_written) overflow = 1; else tmp += (size_t)s_written;
    if (!overflow && tmp > SIZE_MAX - 1) overflow = 1; else tmp += 1; // $
    if (!overflow && tmp > SIZE_MAX - (size_t)h_written) overflow = 1; else tmp += (size_t)h_written;
    if (!overflow && tmp > SIZE_MAX - 1) overflow = 1; else tmp += 1; // null

    if (overflow) {
        free(salt_b64);
        free(hash_b64);
        OPENSSL_cleanse(dk, DK_LEN);
        return NULL;
    }
    total_len = tmp;

    char* record = (char*)malloc(total_len);
    if (!record) {
        free(salt_b64);
        free(hash_b64);
        OPENSSL_cleanse(dk, DK_LEN);
        return NULL;
    }

    int written = snprintf(record, total_len, "%s%s$%s$%s", prefix, iter_buf, salt_b64, hash_b64);
    free(salt_b64);
    free(hash_b64);

    if (written < 0 || (size_t)written >= total_len) {
        free(record);
        OPENSSL_cleanse(dk, DK_LEN);
        return NULL;
    }

    OPENSSL_cleanse(dk, DK_LEN);
    return record;
}

int main(void) {
    const char* tests[5][2] = {
        {"alice_01", "S3cure!Passw0rd"},
        {"bob-user", "An0ther$Strong1"},
        {"charlie.test", "Ultr@S3curePass!"},
        {"dora-2025", "G00d#Password_++"},
        {"eve.user", "Y3t@N0ther_Strong"}
    };

    for (int i = 0; i < 5; i++) {
        char* rec = register_user(tests[i][0], tests[i][1]);
        if (rec) {
            printf("Registered: %s -> %s\n", tests[i][0], rec);
            free(rec);
        } else {
            printf("Registration failed for %s\n", tests[i][0]);
        }
    }
    return 0;
}