#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

#define SALT_LEN 16
#define DERIVED_KEY_LEN 32
#define PBKDF2_ITERATIONS 210000

#define USERNAME_MIN 3
#define USERNAME_MAX 32

#define DB_MAX_USERS 100
#define REC_MAX 256  // base64 of 48 bytes -> 64 chars, keep margin

typedef struct {
    char username[USERNAME_MAX + 1];
    char record[REC_MAX]; // base64(salt||hash)
} user_record;

static user_record DB[DB_MAX_USERS];
static size_t DB_COUNT = 0;

static void secure_zero(void *v, size_t n) {
#if defined(__STDC_LIB_EXT1__)
    memset_s(v, n, 0, n);
#else
    if (v == NULL) return;
    OPENSSL_cleanse(v, n);
#endif
}

static bool is_valid_username(const char *u) {
    if (u == NULL) return false;
    size_t len = strlen(u);
    if (len < USERNAME_MIN || len > USERNAME_MAX) return false;
    for (size_t i = 0; i < len; i++) {
        char c = u[i];
        if (!(isalnum((unsigned char)c) || c == '_' || c == '.' || c == '-')) {
            return false;
        }
    }
    return true;
}

static bool is_strong_password(const char *p) {
    if (p == NULL) return false;
    size_t len = strlen(p);
    if (len < 12) return false;
    bool has_upper=false, has_lower=false, has_digit=false, has_special=false;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)p[i];
        if (isspace(c)) return false;
        if (isupper(c)) has_upper = true;
        else if (islower(c)) has_lower = true;
        else if (isdigit(c)) has_digit = true;
        else has_special = true;
    }
    return has_upper && has_lower && has_digit && has_special;
}

static int base64_encode(const unsigned char *in, size_t in_len, char *out, size_t out_size) {
    int needed = 4 * ((int)in_len + 2) / 3;
    if ((size_t)(needed + 1) > out_size) return -1;
    int enc = EVP_EncodeBlock((unsigned char*)out, in, (int)in_len);
    if (enc < 0) return -1;
    out[enc] = '\0';
    return enc;
}

static int base64_decode(const char *in, unsigned char *out, size_t out_size, size_t *out_len) {
    int in_len = (int)strlen(in);
    if (in_len % 4 != 0) return -1;
    int max_out = 3 * (in_len / 4);
    if ((size_t)max_out > out_size) return -1;
    int dec = EVP_DecodeBlock(out, (const unsigned char*)in, in_len);
    if (dec < 0) return -1;
    int pad = 0;
    if (in_len >= 2) {
        if (in[in_len - 1] == '=') pad++;
        if (in[in_len - 2] == '=') pad++;
    }
    dec -= pad;
    if (dec < 0) dec = 0;
    *out_len = (size_t)dec;
    return 0;
}

static int generate_salt(unsigned char *salt, size_t len) {
    if (len < 16 || len > 64) return -1;
    if (RAND_bytes(salt, (int)len) != 1) return -1;
    return 0;
}

static int hash_password(const char *password, const unsigned char *salt, size_t salt_len,
                         unsigned char *out, size_t out_len) {
    if (out_len < DERIVED_KEY_LEN) return -1;
    const EVP_MD *md = EVP_sha256();
    if (PKCS5_PBKDF2_HMAC(password, (int)strlen(password),
                          salt, (int)salt_len,
                          PBKDF2_ITERATIONS, md,
                          (int)DERIVED_KEY_LEN, out) != 1) {
        return -1;
    }
    return 0;
}

static int find_user_index(const char *username) {
    for (size_t i = 0; i < DB_COUNT; i++) {
        if (strncmp(DB[i].username, username, USERNAME_MAX) == 0) {
            return (int)i;
        }
    }
    return -1;
}

bool register_user(const char *username, const char *password) {
    if (!is_valid_username(username) || !is_strong_password(password)) return false;
    if (find_user_index(username) >= 0) return false;
    if (DB_COUNT >= DB_MAX_USERS) return false;

    unsigned char salt[SALT_LEN];
    unsigned char dk[DERIVED_KEY_LEN];
    unsigned char combined[SALT_LEN + DERIVED_KEY_LEN];
    char b64[REC_MAX];

    if (generate_salt(salt, SALT_LEN) != 0) return false;
    if (hash_password(password, salt, SALT_LEN, dk, DERIVED_KEY_LEN) != 0) {
        secure_zero(salt, sizeof(salt));
        return false;
    }
    memcpy(combined, salt, SALT_LEN);
    memcpy(combined + SALT_LEN, dk, DERIVED_KEY_LEN);

    int enc = base64_encode(combined, sizeof(combined), b64, sizeof(b64));
    if (enc < 0) {
        secure_zero(salt, sizeof(salt));
        secure_zero(dk, sizeof(dk));
        secure_zero(combined, sizeof(combined));
        return false;
    }

    strncpy(DB[DB_COUNT].username, username, USERNAME_MAX);
    DB[DB_COUNT].username[USERNAME_MAX] = '\0';
    strncpy(DB[DB_COUNT].record, b64, REC_MAX - 1);
    DB[DB_COUNT].record[REC_MAX - 1] = '\0';
    DB_COUNT++;

    secure_zero(salt, sizeof(salt));
    secure_zero(dk, sizeof(dk));
    secure_zero(combined, sizeof(combined));
    secure_zero(b64, sizeof(b64)); // not strictly necessary, but avoid lingering
    return true;
}

bool verify_user(const char *username, const char *password) {
    int idx = find_user_index(username);
    if (idx < 0) return false;

    unsigned char combined[SALT_LEN + DERIVED_KEY_LEN];
    size_t combined_len = 0;
    if (base64_decode(DB[idx].record, combined, sizeof(combined), &combined_len) != 0) {
        return false;
    }
    if (combined_len != (SALT_LEN + DERIVED_KEY_LEN)) {
        secure_zero(combined, sizeof(combined));
        return false;
    }
    unsigned char candidate[DERIVED_KEY_LEN];
    if (hash_password(password, combined, SALT_LEN, candidate, DERIVED_KEY_LEN) != 0) {
        secure_zero(combined, sizeof(combined));
        return false;
    }
    bool equal = (CRYPTO_memcmp(combined + SALT_LEN, candidate, DERIVED_KEY_LEN) == 0);
    secure_zero(combined, sizeof(combined));
    secure_zero(candidate, sizeof(candidate));
    return equal;
}

int main(void) {
    // Test 1: valid registration
    bool t1 = register_user("alice", "Str0ng!Passw0rd");
    printf("Test1 register valid: %s\n", t1 ? "true" : "false");

    // Test 2: invalid username
    bool t2 = register_user("a", "Another$trong123");
    printf("Test2 invalid username rejected: %s\n", (!t2) ? "true" : "false");

    // Test 3: weak password
    bool t3 = register_user("bob", "weakpass");
    printf("Test3 weak password rejected: %s\n", (!t3) ? "true" : "false");

    // Test 4: duplicate username
    bool t4a = register_user("carol", "V3ry$trongPass!");
    bool t4b = register_user("carol", "Diff3rent$trong!");
    printf("Test4 duplicate username rejected: %s\n", (t4a && !t4b) ? "true" : "false");

    // Test 5: same password yields different stored records due to unique salts
    bool r1 = register_user("dave", "SamePassw0rd!!");
    bool r2 = register_user("erin", "SamePassw0rd!!");
    bool same_stored = false;
    if (r1 && r2) {
        int i1 = find_user_index("dave");
        int i2 = find_user_index("erin");
        if (i1 >= 0 && i2 >= 0) {
            same_stored = (strncmp(DB[i1].record, DB[i2].record, REC_MAX) == 0);
        }
    }
    bool v1 = verify_user("dave", "SamePassw0rd!!");
    bool v2 = verify_user("erin", "SamePassw0rd!!");
    printf("Test5 unique salts and verify: %s\n", (r1 && r2 && !same_stored && v1 && v2) ? "true" : "false");

    return 0;
}